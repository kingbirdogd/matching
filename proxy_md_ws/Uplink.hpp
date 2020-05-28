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
    uint32_t port, zmq_ob_snapshot_port, zmq_ob_diff_port;
    zmq::context_t ctx;
    zmq::socket_t zmq_ob_snapshot_sock;
    zmq::socket_t zmq_ob_diff_sock;
    std::chrono::steady_clock::time_point next_broadcast_time;
    uint32_t broadcast_ms;
    static std::chrono::steady_clock::duration broadcast_interval; // = std::chrono::milliseconds (broadcast_ms);

    OrderBook ob;

  public:
    Uplink(const char host[], Selector &selector_in, Selector &selector_out, uint8_t num_queue,
        uint32_t matching_port, uint32_t zmq_ob_snapshot_port, uint32_t zmq_ob_diff_port, uint32_t broadcast_ms)
      : host(host), selector_in(selector_in),
        selector_out(selector_out), num_queue(num_queue), port(matching_port),
        ctx(1), zmq_ob_snapshot_sock(ctx, ZMQ_PUB), zmq_ob_diff_sock(ctx, ZMQ_PUB),
        broadcast_ms(broadcast_ms)  {
      Uplink::broadcast_interval = std::chrono::milliseconds (broadcast_ms);
      std::string zmq_ob_snapshot_url(std::string("tcp://*:") + std::to_string(zmq_ob_snapshot_port));
      zmq_ob_snapshot_sock.bind(zmq_ob_snapshot_url);
      elog.debug() << "zmq orderbook snapshot pub socket bind to " << zmq_ob_snapshot_url << std::endl;

      std::string zmq_ob_diff_url(std::string("tcp://*:") + std::to_string(zmq_ob_diff_port));
      zmq_ob_diff_sock.bind(zmq_ob_diff_url);
      elog.debug() << "zmq orderbook diff pub socket bind to " << zmq_ob_diff_url << std::endl;

//      zmq_pub_sock.connect("tcp://localhost:14002"); // hard code for testing
//      std::cout << "Connected to xsub" << std::endl;

      reschedule_broadcast();
      schedule_broadcast();
    }

    void enqueue_work(std::function<void(void) /* noexcept */> &&work);

    void connect(std::function<void(void)> synchronized = nullptr);
    void disconnect();
    void reconnect();

    uint8_t protocol_version() const ;

    bool send_message(const void *msg, size_t n, bool force = false) ;

    bool do_request(const void *msg, size_t n, std::shared_ptr<Client> client_ptr);
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
