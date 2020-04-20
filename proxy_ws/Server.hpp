#include "common/wsserver.h"
#include "common/log.h"
#include "common/dns.h"
#include "Client.hpp"

#ifndef ENGINE_SERVER_HPP
#define ENGINE_SERVER_HPP

extern Log elog;

namespace proxy {
  //Custom errors for HTTP request headers validation
  const char REQUIRED_HEADER_NOT_FOUND_CUSTOM_ERR_MSG[] = "Required HTTP header not found";
  const char EXCEEDED_CONCURRENT_CONNECTIONS_LIMIT_CUSTOM_ERR_MSG[] = "Exceeded concurrent connections limit";

  class Server : public WebSocketServer {
  public:
    static constexpr in_port_t DEFAULT_PORT = 8080;
    //static constexpr uint      DEFAULT_MIN_ORDER_QTY = 1;

  private:
    static Socket6 create_socket(in_port_t port) {
      Socket6 socket(SOCK_STREAM);
      socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);
      socket.setsockopt(IPPROTO_TCP, TCP_FASTOPEN, SOMAXCONN);
      sockaddr_in6 addr;
      addr.sin6_family = AF_INET6;
      as_be(addr.sin6_port) = port;
      addr.sin6_flowinfo = 0;
      addr.sin6_addr = in6addr_any;
      addr.sin6_scope_id = 0;
      socket.bind(addr);
      socket.listen();
      return socket;
    }

    uint8_t num_queue;

  public:
    Server(const std::string &ip_address_identifier, int max_concurrent_connections, in_port_t port = DEFAULT_PORT, uint8_t num_queue = 1)
        : WebSocketServer(create_socket(port)),
          _ip_address_identifier(ip_address_identifier),
          _max_concurrent_connections(max_concurrent_connections),
          num_queue(num_queue) {
      if (elog.info_enabled()) {
        sockaddr_in6 addr;
        socklen_t addrlen = sizeof addr;
        this->getsockname(reinterpret_cast<sockaddr *>(&addr), &addrlen);
        elog.info() << "listening on " << addr << ", maximum concurrent connections per IP address: " << _max_concurrent_connections << std::endl;
      }
    }

  protected:
    status_t _pure validate_request_headers(const HttpRequestHeaders &request_headers) override {
      if (request_headers.request_uri != "/v1" && request_headers.request_uri != "/") {
        return {404, HTTP_REASON_PHRASE_404};
      }

      std::pair<bool, std::string> ret = _retrieve_client_ip_address(request_headers);

      if (!ret.first) {
        return {404, REQUIRED_HEADER_NOT_FOUND_CUSTOM_ERR_MSG};
      }

      const std::string client_ip_address = ret.second;

      //Check if the ip address is already in the map
      {
        std::lock_guard<std::mutex> _guard(_client_connections_map_mutex);

        auto connections_map_itr = _client_connections_map.find(client_ip_address);
        if (connections_map_itr != std::end(_client_connections_map)) {
          //map contains already that ip address - check against the limit
          const int current_connections = connections_map_itr->second;
          if (current_connections < _max_concurrent_connections) {
            //We can still create extra connection
            connections_map_itr->second += 1;

            if (elog.debug_enabled()) {
              elog.debug() << "Updating concurrent connections count for IP address: " << client_ip_address << " to: " << connections_map_itr->second << std::endl;
            }
          } else {
            //No more connections allowed, reject
            if (elog.debug_enabled()) {
              elog.debug() << "No more concurrent connections allowed for IP address: " << client_ip_address << ". Connection denied" << std::endl;
            }

            return {404, EXCEEDED_CONCURRENT_CONNECTIONS_LIMIT_CUSTOM_ERR_MSG};
          }
        } else {
          //map does not contain the ip address - add it
          if (elog.debug_enabled()) {
            elog.debug() << "Updating concurrent connections count for IP address: " << client_ip_address << " to: 1" << std::endl;
          }
          _client_connections_map.insert(std::make_pair(client_ip_address, 1));
        }
      }

      return {101, HTTP_REASON_PHRASE_101};
    }

    void prepare_response_headers(const HttpRequestHeaders &, HttpResponseHeaders &response_headers) override {
      response_headers.emplace("Access-Control-Allow-Origin", "*");
    }

    void client_attached(Socket &&socket, Selector &selector, const HttpRequestHeaders &request_headers) override {
      sockaddr_in6 peer_addr;
      socklen_t addrlen = sizeof peer_addr;
      socket.getpeername(reinterpret_cast<sockaddr *>(&peer_addr), &addrlen);

      const std::string ip_address = _retrieve_client_ip_address(request_headers).second;

      if (elog.debug_enabled()) {
        elog.debug() << "accepted connection from " << ip_address << std::endl;
      }
      //auto client_ptr = new Client(std::move(socket), peer_addr, request_headers.request_uri == "/v1" ? 1 : 0);
      auto client_ptr = new Client(std::move(socket), peer_addr, request_headers.request_uri == "/v1" ? 1 : 0, num_queue, ip_address, _client_connections_map, _client_connections_map_mutex);
      selector.modify(client_ptr->socket, client_ptr, Selector::Flags::READABLE);
    }

  private:
    /** Retrieve client ip address based on the identifier used
     *  std::pair<bool, std::string> - First denotes if we have successfully retrieved ip address and should proceed, second contains ip address, otherwise empty string
    */
    std::pair<bool, std::string> _retrieve_client_ip_address(const HttpRequestHeaders &request_headers) {

      auto ip_addr_itr = request_headers.find(_ip_address_identifier);
      if (ip_addr_itr == std::end(request_headers)) {
        if (elog.debug_enabled()) {
          elog.debug() << "Failed to find HTTP header field: " << _ip_address_identifier << ". Connection denied" << std::endl;
        }
        //Failed to find ip address
        return {false, ""};
      }

      if (_ip_address_identifier == CF_IP_ADDRESS_IDENTIFIER) {
        //return the associated value
        return {true, ip_addr_itr->second};
      } else if (_ip_address_identifier == AWS_IP_ADDRESS_IDENTIFIER) {
        const std::string ip = ip_addr_itr->second;

        //X-Forwarded-For could return a list of items in which case we pick the left most value
        auto pos = ip.find(',');
        if (pos == std::string::npos) {
          //no comma, return as is
          return {true, ip};
        } else {
          return {true, ip.substr(0, pos)};
        }
      } else {
        if (elog.debug_enabled()) {
          elog.debug() << "IP address identifier " << _ip_address_identifier << " does not match expected values of " << CF_IP_ADDRESS_IDENTIFIER << " or " << AWS_IP_ADDRESS_IDENTIFIER << ". Connection denied" << std::endl;
        }

        //We have not found ip address
        return {false, ""};
      }
    }

    /** Map that holds number of websocket connections per client
        Key - IP address of the client
        Value - number of connections
    */
    std::unordered_map<std::string, int> _client_connections_map;

    /** Mutex for connections map
    */
    std::mutex _client_connections_map_mutex;

    /** Should only hold a value of "X-Forwarded-For" or "CF-Connecting-IP"
    */
    std::string _ip_address_identifier;

    /** IP address identifiers for different load balancers
    */
    const std::string AWS_IP_ADDRESS_IDENTIFIER = "X-Forwarded-For";
    const std::string CF_IP_ADDRESS_IDENTIFIER = "CF-Connecting-IP";

    /** Maximum concurrent connections allowed per ip address
    */
    int _max_concurrent_connections;
  };
}
#endif //ENGINE_SERVER_HPP
