#ifndef ENGINE_UPLINK_HPP
#define ENGINE_UPLINK_HPP

#include <matching/order.hpp>
#include <zmq.hpp>
#include "common/log.h"
#include "transceiver.h"
#include "workqueue.h"
//#include "Client.hpp"
#include "common/connect.h"
#include <map>
#include <md/book_item.hpp>
#include <common/json.h>
#include "OrderBook.hpp"
#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/idl.h>
#include <pulsar/Client.h>

using namespace pulsar;

extern Log elog;

namespace proxy {
  using core::Transceiver;
  class Client;

  struct UplinkConfig {
    std::string host;
    uint8_t num_queue;
    uint32_t matching_port;
  };

  class Uplink : public Transceiver, public WorkQueue {
  public:
    using book_map_t = std::map<long long, md::book_item>;
  private:
    const char *const host;
    Selector &selector_in, &selector_out;

    Buffer send_buf;

    std::function<void(void)> synchronized;

    mutable std::mutex callback_queue_mutex;
    std::queue<std::shared_ptr<Client>> callback_queue;

    uint8_t num_queue;
    uint32_t port; //, zmq_ob_snapshot_port, zmq_ob_diff_port;
    //zmq::context_t ctx;
    //zmq::socket_t zmq_ob_snapshot_sock;
    //zmq::socket_t zmq_ob_diff_sock;
    std::chrono::steady_clock::time_point next_broadcast_time;
    uint32_t broadcast_ms;
    static std::chrono::steady_clock::duration broadcast_interval; // = std::chrono::milliseconds (broadcast_ms);

    OrderBook ob;
    flatbuffers::Parser parser;
    pulsar::Client client;
    Producer prd_snapshot;
    Producer prd_diff;

  public:
    Uplink(const char host[], Selector &selector_in, Selector &selector_out, uint8_t num_queue,
        uint32_t matching_port, //uint32_t zmq_ob_snapshot_port, uint32_t zmq_ob_diff_port,
        std::string pulsar_host_url_option, std::string md_pub_snapshot_url, std::string md_pub_diff_url,
           uint32_t broadcast_ms, std::string md_schema_file)
      : host(host), selector_in(selector_in),
        selector_out(selector_out), num_queue(num_queue), port(matching_port),
        client(pulsar_host_url_option),
        //ctx(1), zmq_ob_snapshot_sock(ctx, ZMQ_PUB), zmq_ob_diff_sock(ctx, ZMQ_PUB),
        broadcast_ms(broadcast_ms)  {
      elog.info() << "Pulsar Host: " << pulsar_host_url_option << std::endl;
      elog.info() << "Creating Producer: " << md_pub_snapshot_url << std::endl;
      Result result = client.createProducer(md_pub_snapshot_url, prd_snapshot);
      if (result != ResultOk) {
        elog.error() << "Error creating producer: " << result;
      }
      elog.info() << "Creating Producer: " << md_pub_diff_url << std::endl;
      result = client.createProducer(md_pub_diff_url, prd_diff);
      if (result != ResultOk) {
        elog.error() << "Error creating producer: " << result;
      }

      Uplink::broadcast_interval = std::chrono::milliseconds (broadcast_ms);
//      std::string zmq_ob_snapshot_url(std::string("tcp://*:") + std::to_string(zmq_ob_snapshot_port));
//      zmq_ob_snapshot_sock.bind(zmq_ob_snapshot_url);
//      elog.debug() << "zmq orderbook snapshot pub socket bind to " << zmq_ob_snapshot_url << std::endl;
//
//      std::string zmq_ob_diff_url(std::string("tcp://*:") + std::to_string(zmq_ob_diff_port));
//      zmq_ob_diff_sock.bind(zmq_ob_diff_url);
//      elog.debug() << "zmq orderbook diff pub socket bind to " << zmq_ob_diff_url << std::endl;

//      zmq_pub_sock.connect("tcp://localhost:14002"); // hard code for testing
//      std::cout << "Connected to xsub" << std::endl;

      reschedule_broadcast();
      schedule_broadcast();

      std::string schema_ok_file;
      bool ok = flatbuffers::LoadFile(md_schema_file.c_str(), false, &schema_ok_file);
      if (!ok) {
        std::cout << "load file failed!" << std::endl;
        return;
      }
      parser.Parse(schema_ok_file.c_str());
    }

    void pulsar_broadcast(const json::Object &jo, Producer &prd);
    void enqueue_work(std::function<void(void) /* noexcept */> &&work);

    void connect(std::function<void(void)> synchronized = nullptr);
    void disconnect();
    void reconnect();

    uint8_t protocol_version() const ;

    bool send_message(const void *msg, size_t n, bool force = false) ;

    bool do_request(const void *msg, size_t n, std::shared_ptr<proxy::Client> client_ptr);
    void schedule_broadcast() noexcept;
    void reschedule_broadcast() noexcept;

    //json::Object handle_book_item(const md::book_item& o);
    json::Object get_orderbook_snapshot(book_map_t &bids, book_map_t &asks);
    json::Object get_orderbook_diff(book_map_t &bids, book_map_t &asks, book_map_t &last_bids, book_map_t &last_asks);

  protected:
    void selected(Selector &selector, Selector::Flags flags) noexcept override ;

    void release() noexcept override ;

    size_t receive_message(const void *buf, size_t n) override;

  };

}

#endif //ENGINE_UPLINK_HPP
