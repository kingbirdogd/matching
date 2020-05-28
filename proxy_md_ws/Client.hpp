#pragma once
#ifndef ENGINE_CLIENT_HPP
#define ENGINE_CLIENT_HPP

#include "common/websocket.h"
#include "common/ratelimit.h"
#include "common/json.h"
#include "common/log.h"
#include "common/dns.h"
#include "common/sha.h"
#include "common/ecp.h"
#include "common/scheduler.h"
#include "common/selector.h"
#include "core.h"
#include "workqueue.h"
//#include "Book.hpp"
#include "User.hpp"
#include "Uplink.hpp"
#include "proxy_lws_utils.hpp"
#include "libwebsockets.h"
#include "proxy_lws_struct.hpp"
//#include "contrib/concurrentqueue/concurrentqueue.h"
#include <shared_mutex>
#include <list>
#include <set>
#include <thread>
#include <sys/ioctl.h>
#include "folly/concurrency/UnboundedQueue.h"

extern Log elog;
//using namespace moodycamel;
//using namespace folly;

namespace proxy {
  using namespace core;
  using core::id_t;
  class Uplink;

  extern bool fee_control_allowed;
  extern bool skip_auth_allowed  ;
  extern Selector *selector;
  extern Uplink *uplink;
  extern Scheduler<std::chrono::steady_clock> scheduler;

  //class Client : public WebSocket, public Selectable, public WorkQueue {
class Client : public Selectable, public WorkQueue { //, public std::enable_shared_from_this<Client>

  public:
    static constexpr std::chrono::steady_clock::duration ping_interval = std::chrono::seconds(45);
    static std::list<Client *> all_clients;
  private:
    static std::shared_mutex clients_rwlock;
    static std::multimap<id_t, Client *> users_clients;

  public:
    folly::UMPSCQueue<json::Object, true, 10>  reply_queue;
    //ConcurrentQueue<                json::Object>  reply_queue;
    //ConcurrentQueue<std::shared_ptr<std::string_view>> broadcast_queue;
    struct per_vhost_data__minimal *vhd;

//    std::shared_ptr<Client> get_shared_from_this() {
//
//      return this->std::enable_shared_from_this<Client>::shared_from_this();
//      //return Client::shared_from_this();
//    }

    static void disconnect_user(id_t user_id) {
//      std::shared_lock<std::shared_mutex> clients_rdlock(clients_rwlock);
//      auto range = users_clients.equal_range(user_id);
//      for (auto itr = range.first; itr != range.second; ++itr) {
//        try {
//          itr->second->socket.shutdown(SHUT_RDWR);
//        }
//        catch (...) {
//          continue;
//        }
//      }
    }

    template<typename Formatter>
    static std::enable_if_t<!std::is_convertible_v<Formatter, const json::Value &>> broadcast(const Formatter &formatter) {
      if (elog.debug_enabled()) {
        auto msg = formatter(~id_t(), ~uint8_t());
        if (!msg.empty()) {
          elog.debug() << "!! " << msg << std::endl;
        }
      }
      std::shared_lock<std::shared_mutex> clients_rdlock(clients_rwlock);
      multicast(all_clients, formatter);
    }

//    static void broadcast(const json::Value &msg) {
//      broadcast(CachingFormatter<json::Value>(msg));
//    }

    static void broadcast(const json::Object &msg) {
      if (elog.debug_enabled()) {
        std::stringstream ss;
        ss << msg;
        elog.debug() << "!! << " << ss.str() << std::endl;
      }

      std::shared_lock<std::shared_mutex> clients_rdlock(clients_rwlock);
      multicast(all_clients, msg);
    }


//    template<typename Formatter>
//    static std::enable_if_t<!std::is_convertible_v<Formatter, const json::Value &>> multicast(id_t user_id, const Formatter &formatter) {
//      if (elog.trace_enabled()) {
//        std::thread::id this_id = std::this_thread::get_id();
//        auto msg = formatter(user_id, ~uint8_t());
//        if (!msg.empty()) {
//          elog.trace() << "Thread(" << this_id << ") " << user_id << " << " << msg << std::endl;
//        }
//      } else if (elog.debug_enabled()) {
//        auto msg = formatter(user_id, ~uint8_t());
//        if (!msg.empty()) {
//          elog.debug() << user_id << " << " << msg << std::endl;
//        }
//      }
//
//      std::shared_lock<std::shared_mutex> clients_rdlock(clients_rwlock);
//      auto range = users_clients.equal_range(user_id);
//      for (auto itr = range.first; itr != range.second; ++itr) {
//        try {
//          auto msg = formatter(itr->second->user_id, itr->second->api_version);
//          if (!msg.empty()) {
//            if (elog.debug_enabled()) {
//              elog.debug() << "Before send: " << user_id << " << " << msg << std::endl;
//            }
//            itr->second->reply_queue.enqueue(msg);
//            lws_callback_on_writable(itr->second->wsi);
//            //std::lock_guard<std::mutex> send_lock(itr->second->send_mutex);
//            //itr->second->send(Text, msg.data(), msg.size());
//          }
//        }
//        catch (...) {
//          continue;
//        }
//      }
//    }

//    static void multicast(id_t user_id, const json::Value &msg) {
//      if (elog.debug_enabled()) {
//        std::stringstream ss;
//        ss << msg;
//        elog.debug() << "Multicast original msg: " << user_id << " << " << ss.str() << std::endl;
//      }
//      multicast(user_id, CachingFormatter<json::Value>(msg));
//    }

    static void multicast(id_t user_id, const json::Object &msg) {
      if (elog.debug_enabled()) {
        std::stringstream ss;
        ss << msg;
        elog.debug() << user_id << " << " << ss.str() << std::endl;
      }

      std::shared_lock<std::shared_mutex> clients_rdlock(clients_rwlock);
      auto range = users_clients.equal_range(user_id);
      for (auto itr = range.first; itr != range.second; ++itr) {
        try {
          if (!itr->second->wsi) continue;
          if (elog.debug()) elog.debug() << std::hex << "enqueuing wsi " <<  itr->second->wsi << std::endl;
          //itr->second->reply_queue.enqueue(itr->second->ptok,msg);
          //itr->second->vhd->wsi_queue.enqueue(itr->second->ptok,itr->second->wsi);
          itr->second->reply_queue.enqueue(msg);
          itr->second->vhd->wsi_queue->enqueue(itr->second->wsi);
          lws_cancel_service(lws_get_context(itr->second->wsi));
          //lws_cancel_service(itr->second->vhd->context);
          //lws_callback_on_writable(itr->second->wsi);
        }
        catch (...) {
          continue;
        }
      }
    }

//    template<typename Formatter>
//    static std::enable_if_t<!std::is_convertible_v<Formatter, const json::Value &>> multicast(const std::list<Client *> &clients, const Formatter &formatter) {
//      if (elog.debug_enabled()) {
//        auto msg = formatter(~id_t(), ~uint8_t());
//        if (!msg.empty()) {
//          elog.debug() << "!! " << msg << std::endl;
//        }
//      }
//      for (Client *client_ptr : clients) {
//        try {
//          auto msg = formatter(client_ptr->user_id, client_ptr->api_version);
//          if (!msg.empty()) {
//            client_ptr->reply_queue.enqueue(msg);
//            lws_callback_on_writable(client_ptr->wsi);
//            //std::lock_guard<std::mutex> send_lock(client_ptr->send_mutex);
//            //client_ptr->send(Text, msg.data(), msg.size());
//          }
//        }
//        catch (...) {
//          continue;
//        }
//      }
//    }

//    static void multicast(const std::list<Client *> &clients, const json::Value &msg) {
//      multicast(clients, CachingFormatter<json::Value>(msg));
//    }
//    static void multicast(const std::list<Client *> &clients, const json::Value &msg, id_t exclude_user_id) {
//      multicast(clients, CachingFormatter<json::Value, ExcludeUser>(msg, exclude_user_id));
//    }
//    static void multicast(const std::list<Client *> &clients, const json::Value &msg, id_t exclude_user_id1, id_t exclude_user_id2) {
//      multicast(clients, CachingFormatter<json::Value, ExcludeUsers>(msg, ExcludeUsers({exclude_user_id1, exclude_user_id2})));
//    }

    static void multicast(const std::list<Client *> &clients, const json::Object &msg) {
      for (Client *client_ptr : clients) {
        try {
            if (!client_ptr->wsi) continue;
            if (elog.debug()) elog.debug() << std::hex << "enqueuing wsi " <<  client_ptr->wsi << std::endl;
//            client_ptr->reply_queue.enqueue(client_ptr->ptok,msg);
//            client_ptr->vhd->wsi_queue.enqueue(client_ptr->ptok,client_ptr->wsi);
            client_ptr->reply_queue.enqueue(msg);
            client_ptr->vhd->wsi_queue->enqueue(client_ptr->wsi);
            lws_cancel_service(lws_get_context(client_ptr->wsi));
            //lws_cancel_service(client_ptr->vhd->context);
            //lws_callback_on_writable(client_ptr->wsi);
        }
        catch (...) {
          continue;
        }
      }
    }

    static void multicast_orderbook(const std::list<Client *> &clients, const json::Object &snapshot, const json::Object &delta) {
      for (Client *client_ptr : clients) {
        try {
          if (!client_ptr->wsi) continue;
          if (elog.debug()) elog.debug() << std::hex << "enqueuing wsi " <<  client_ptr->wsi << std::endl;
          if (client_ptr->snapshot_sent)
            client_ptr->reply_queue.enqueue(delta);
          else
            client_ptr->reply_queue.enqueue(snapshot);
          client_ptr->vhd->wsi_queue->enqueue(client_ptr->wsi);
          lws_cancel_service(lws_get_context(client_ptr->wsi));
          client_ptr->snapshot_sent = true;
        }
        catch (...) {
          continue;
        }
      }
    }

    static void multicast(const std::list<Client *> &clients, const json::Object &msg, id_t exclude_user_id) {
      for (Client *client_ptr : clients) {
        try {
          if (client_ptr->get_client_user_id() != exclude_user_id) {
            if (!client_ptr->wsi) continue;
            if (elog.debug()) elog.debug() << std::hex << "enqueuing wsi " <<  client_ptr->wsi << std::endl;
//            client_ptr->reply_queue.enqueue(client_ptr->ptok,msg);
//            client_ptr->vhd->wsi_queue.enqueue(client_ptr->ptok,client_ptr->wsi);
            client_ptr->reply_queue.enqueue(msg);
            client_ptr->vhd->wsi_queue->enqueue(client_ptr->wsi);
            lws_cancel_service(lws_get_context(client_ptr->wsi));
            //lws_cancel_service(client_ptr->vhd->context);
            //lws_callback_on_writable(client_ptr->wsi);
          }
        }
        catch (...) {
          continue;
        }
      }
    }

    static void multicast(const std::list<Client *> &clients, const json::Object &msg, id_t exclude_user_id1, id_t exclude_user_id2) {
      for (Client *client_ptr : clients) {
        try {
          if ((client_ptr->get_client_user_id() != exclude_user_id1) && (client_ptr->get_client_user_id() != exclude_user_id2)) {
            if (!client_ptr->wsi) continue;
            if (elog.debug()) elog.debug() << std::hex << "enqueuing wsi " <<  client_ptr->wsi << std::endl;
//            client_ptr->reply_queue.enqueue(client_ptr->ptok,msg);
//            client_ptr->vhd->wsi_queue.enqueue(client_ptr->ptok,client_ptr->wsi);
            client_ptr->reply_queue.enqueue(msg);
            client_ptr->vhd->wsi_queue->enqueue(client_ptr->wsi);
            lws_cancel_service(lws_get_context(client_ptr->wsi));
            //lws_cancel_service(client_ptr->vhd->context);
            //lws_callback_on_writable(client_ptr->wsi);
          }
        }
        catch (...) {
          continue;
        }
      }
    }

    static void clear_order(id_t user_id, id_t order_id) {
      std::shared_lock<std::shared_mutex> clients_rdlock(clients_rwlock);
      auto range = users_clients.equal_range(user_id);
      for (auto itr = range.first; itr != range.second; ++itr) {
        std::lock_guard<std::mutex> transient_orders_lock(itr->second->transient_orders_mutex);
        if (itr->second->transient_orders.erase(order_id)) {
          break;
        }
      }
    }

  private:
    std::array<uint8_t, 1024> recv_buf;
    unsigned recv_mark = 0, recv_pos = 0;
    Opcode recv_opcode;
    bool fragmented = false, throttled = false;
    bool snapshot_sent = false;

  public:
    const uint8_t api_version;
    std::shared_ptr<Client> shared_this;

  private:
    std::mutex send_mutex;
    //sockaddr_in6 peer_addr;
    std::string peer_addr;
    std::chrono::steady_clock::time_point next_ping_time;
    std::list<Client *>::iterator all_clients_itr;
    id_t user_id;
    uint8_t nonce[16];
    TokenBucket<std::chrono::steady_clock, std::ratio<1000, 3600>, 1000> auth_bucket;
    TokenBucket<std::chrono::steady_clock, std::ratio<1, 1>, 10> info_bucket;
    TokenBucket<std::chrono::steady_clock, std::ratio<200, 1>, 200> order_bucket;

    std::mutex transient_orders_mutex;
    std::set<id_t> transient_orders;

    mutable std::mutex callback_queue_mutex;
    std::queue<std::function<size_t(const void *, size_t)>> callback_queue;

    std::mutex asset_authority_mutex;
    std::unique_ptr<std::set<asset_t>> asset_authority;

    std::map<asset_pair_t, std::list<Client *>::iterator> orders_watches, ticker_watches;
    uint8_t num_queue;

    /** Actual IP address of the client
    */
    std::string _ip_address;

    /** Reference to the map maintained by the server
    */
    std::unordered_map<std::string, int> &_client_connections_map;

    /** Reference to the mutex to protect the connections map
    */
    std::mutex &_client_connections_map_mutex;
    struct lws *wsi;
    id_t client_user_id;
    //ProducerToken ptok;

  public:
    void        set_wsi(struct lws *_wsi) { wsi = _wsi; }
    struct lws* get_wsi()                 { return wsi; }

    //Client(Socket &&socket, const sockaddr_in6 &peer_addr, uint8_t api_version, uint8_t num_queue, const std::string &ip_address, std::unordered_map<std::string, int> &client_connections_map, std::mutex &client_connections_map_mutex)
    Client(Socket &&socket, struct lws *_wsi, struct per_vhost_data__minimal *_vhd, const char *peer_addr, uint8_t api_version, uint8_t num_queue, const std::string &ip_address, std::unordered_map<std::string, int> &client_connections_map, std::mutex &client_connections_map_mutex)
        : //WebSocket(std::move(socket), false),
          wsi(_wsi), vhd(_vhd), //ptok(vhd->wsi_queue),
          api_version(api_version), peer_addr(peer_addr), user_id(~id_t()),
          num_queue(num_queue), _ip_address(ip_address),
          _client_connections_map(client_connections_map), _client_connections_map_mutex(client_connections_map_mutex)
          {
      random_fill(nonce);
      {
//        json::Object notice;
//        notice.insert("notice", json::String("Welcome"));
//        notice.insert("nonce", json::String(transcode<Base64Encoder>(nonce, sizeof nonce)));
//        if (api_version < 1) {
//          notice.insert("warning", json::String("You are using a deprecated version of this API. Future support for this version is not guaranteed. Please change your code to use the latest version."));
//        }
//        this->send_message(notice);
      }
      {
        std::lock_guard<std::shared_mutex> clients_wrlock(clients_rwlock);
        all_clients_itr = all_clients.insert(all_clients.end(), this);
      }
      //shared_this.reset(this);
      //this->schedule_ping();
    }

    ~Client() {
      if (elog.debug_enabled()) {
        elog.debug() << "closing connection with ip address: " << _ip_address << std::endl;
      }

//      for (auto &orders_watch : orders_watches) {
//        get_book(orders_watch.first)->remove_orders_client(orders_watch.second);
//      }
//      for (auto &ticker_watch : ticker_watches) {
//        get_book(ticker_watch.first)->remove_ticker_client(ticker_watch.second);
//      }
      this->switch_user();
      std::lock_guard<std::shared_mutex> clients_wrlock(clients_rwlock);
      all_clients.erase(all_clients_itr);

      {
        std::lock_guard<std::mutex> _guard(_client_connections_map_mutex);

        //updating the connections map
        auto it = _client_connections_map.find(_ip_address);
        if (it != std::end(_client_connections_map)) {
          if (it->second == 1) {
            _client_connections_map.erase(it);
            if (elog.debug_enabled()) {
              elog.debug() << "No active connections for ip address: " << _ip_address << std::endl;
            }
          } else {
            it->second -= 1;
            if (elog.debug_enabled()) {
              elog.debug() << "Decrementing a number of connections for ip address " << _ip_address
                           << " to " << it->second << std::endl;
            }
          }
        } else {
          if (elog.debug_enabled()) {
            elog.debug()
                << "Failed to find ip address in the map. Not decrementing a number of connections for ip address: "
                << _ip_address << std::endl;
          }
        }
      }
    }

    void enqueue_work(std::function<void(void) /* noexcept */> &&work) {
      if (num_queue == 1)
        this->enqueue_1_queue(std::move(work));
      else
        this->enqueue(std::move(work));
    }

    size_t receive_response(const void *buf, size_t n) ;

    id_t get_client_user_id()        { return client_user_id; }
    void set_client_user_id(id_t id) { client_user_id = id;   }

  protected:
    void selected(Selector &selector, Selector::Flags flags) noexcept override {
      /*
      if ((flags & Selector::Flags::READABLE) != Selector::Flags::NONE) {
        try {
          do {
            ssize_t r = this->receive(recv_opcode, recv_buf.data() + recv_pos, recv_buf.size() - recv_pos);
            if (r < 0) {
              switch (recv_opcode) {
                case Continuation:
                  if (!fragmented) { // peer tried to continue a message without starting one
                    goto shutdown;
                    case Text:
                      if (fragmented) { // peer started a new message without finishing the last one
                        goto shutdown;
                      }
                  }
                  if (this->is_final()) {
                    fragmented = false;
                    this->received(reinterpret_cast<const char *>(recv_buf.data()), recv_pos);
                    recv_pos = 0;
                  } else {
                    fragmented = true;
                  }
                  recv_mark = recv_pos;
                  break;
                case Ping: {
                  std::lock_guard<std::mutex> send_lock(send_mutex);
                  if (!this->WebSocket::send(Pong, recv_buf.data() + recv_mark, recv_pos - recv_mark)) {
                    goto shutdown;
                  }
                  _fallthrough;
                }
                case Pong:
                  recv_pos = recv_mark;
                  break;
                case Binary:
                case Close:
                case End:
                  goto shutdown;
              }
            } else if (r == 0) {
              if (recv_pos == recv_buf.size()) { // peer is sending a message that's too large
                goto shutdown;
              }
              break;
            } else {
              recv_pos += static_cast<unsigned>(r);
            }
          } while (this->is_frame_fully_received());
          std::lock_guard<std::mutex> callback_queue_lock(callback_queue_mutex);
          if (callback_queue.size() < MAX_REQUESTS) {
            selector.modify(socket, this, Selector::Flags::READABLE);
          } else {
            throttled = true;
          }
          return;
        }
        catch (...) {
        }
        shutdown:
        try {
          socket.shutdown(SHUT_RDWR);
        }
        catch (...) {
        }
        shared_this.reset();
      }
      */
    }

  public:
    void received(const char text[], size_t n) {
      //this->reschedule_ping();
      if (elog.debug_enabled()) {
        (elog.debug() << peer_addr << " >> ").write(text, n) << std::endl;
      }
      json::ValuePtr value;
      MemoryBuf mb(text, n);
      std::istream(&mb) >> value;
      try {
        this->message_received(*value);
      }
      catch (const std::logic_error &e) {
        intmax_t tag = 0;
        try {
          auto request_tag_ptr = value->as_object().find("tag");
          if (request_tag_ptr) {
            tag = *request_tag_ptr->as_integer();
          }
        }
        catch (...) {
        }
        this->send_error(tag, 8, e.what());
      }
    }

  private:
    void Msend(Opcode opcode, const void *buf, size_t n) {
      /*
      if (this->WebSocket::send(opcode, buf, n, false)) {
        this->reschedule_ping();
      } else {
        if (elog.warn_enabled()) {
          int i;
          socket.ioctl(TIOCOUTQ, &i);
          elog.warn() << "dropping " << peer_addr << " with " << i << " bytes queued to send" << std::endl;
        }
        socket.shutdown(SHUT_RDWR);
      }*/
    }

    void send_message(const json::Object &msg) {
      if (!wsi) return;
      if (elog.debug()) elog.debug() << std::hex << "enqueuing wsi " << wsi << std::endl;
      reply_queue.enqueue(msg);
      vhd->wsi_queue->enqueue(wsi);
//      if (elog.debug_enabled()) {
//        elog.debug() << "Calling lws_cancel_service" << std::endl;
//      }
      lws_cancel_service(lws_get_context(wsi));
      //lws_cancel_service(vhd->context);
      //lws_callback_on_writable(wsi);
    }

//    void send_message(const json::Value &msg) {
//      const int N = 128;
//      char buf[LWS_PRE + N];
//      //char PRE_PAD[LWS_PRE];
//      memset(&buf[LWS_PRE], 0, N);
//      std::stringstream ss;
//      ss << msg;
//      int n = lws_snprintf(buf + LWS_PRE, N,"%s", ss.str().c_str());
//
//      if (elog.debug_enabled()) {
//        elog.debug() << peer_addr << " << " << &buf[LWS_PRE] << " len=" << n << std::endl;
//      }
//      lws_callback_on_writable(wsi);
//
//
//      /* notice we allowed for LWS_PRE in the payload already */
//      int m = lws_write(wsi, (unsigned char *)&buf[LWS_PRE],
//      //int m = lws_write(wsi, ((buf) + LWS_PRE),
//                        n, LWS_WRITE_TEXT);
//      //if (m < n) { lwsl_err("ERROR %d writing to ws socket\n", m); }
//
////      std::lock_guard<std::mutex> send_lock(send_mutex);
////      WebSocketBuf wsb(this);
////      std::ostream os(&wsb);
////      os.exceptions(std::ios_base::badbit | std::ios_base::failbit);
////      os << msg << std::flush;
////      this->reschedule_ping();
//    }

    void send_error(intmax_t tag, int code, const std::string &msg) {
      json::Object response;
      if (tag) {
        response.insert("tag", json::Integer(tag));
      }
      response.insert("error_code", json::Integer(code));
      response.insert("error_msg", json::String(msg));
      this->send_message(response);
    }

    template<typename T>
    void parse_post_only_flag(const json::Object &request, T &cmd) {
      auto postonly_param_ptr = request.find("post_only");
      if (postonly_param_ptr) {
        if (*postonly_param_ptr->as_boolean()) {
          cmd.flags |= PlaceOrderEx::POST_ONLY;
        } else {
          throw std::invalid_argument("invalid post_only flag");
        }
      }
    }

    void message_received(const json::Value &msg) ;

    void schedule_ping() noexcept;

    void reschedule_ping() noexcept;

//    template<typename M>
//    void do_request(intmax_t tag, const M &msg, std::function<size_t(const void *, size_t)> &&callback);
    template<typename M>
    void do_request(intmax_t tag, const M &msg, std::function<size_t(const void *, size_t)> &&callback) {
      if (elog.debug_enabled()) {
        elog.debug() << "<< " << msg << std::endl;
      }
      this->do_request(tag, &msg, sizeof msg, std::move(callback));
    }


    void do_request(intmax_t tag, const void *msg, size_t n, std::function<size_t(const void *, size_t)> &&callback);

    void do_privileged(intmax_t tag, asset_t asset_id, std::function<void(void)> &&action);

    void switch_user(id_t user_id = ~id_t());

  };

}
#endif //ENGINE_CLIENT_HPP
