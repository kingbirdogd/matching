#ifndef ENGINE_UPLINK_HPP
#define ENGINE_UPLINK_HPP

#include <matching/order.hpp>
#include <zmq.hpp>
#include "common/log.h"
#include "transceiver.h"
#include "workqueue.h"
//#include "Client.hpp"
#include "common/connect.h"

extern Log elog;

namespace proxy {
  using core::Transceiver;
  class Client;

  class Uplink : public Transceiver, public WorkQueue {

  private:
    const char *const host;
    Selector &selector_in, &selector_out;

    Buffer send_buf;

    std::function<void(void)> synchronized;

    mutable std::mutex callback_queue_mutex;
    std::queue<std::shared_ptr<Client>> callback_queue;

    uint8_t num_queue;
    uint32_t port;
    zmq::context_t ctx;
    zmq::socket_t pub_sock;

  public:
    Uplink(const char host[], Selector &selector_in, Selector &selector_out, uint8_t num_queue, uint32_t matching_port)
      : host(host), selector_in(selector_in),
        selector_out(selector_out), num_queue(num_queue), port(matching_port),
        ctx(1), pub_sock(ctx, ZMQ_PUB) {
      pub_sock.connect("tcp://localhost:14002"); // hard code for testing
      std::cout << "Connecting to xsub on tcp://localhost:14002" << std::endl;
    }

    void enqueue_work(std::function<void(void) /* noexcept */> &&work);

    void connect(std::function<void(void)> synchronized = nullptr);
    void disconnect();
    void reconnect();

    uint8_t protocol_version() const ;

    bool send_message(const void *msg, size_t n, bool force = false) ;

    bool do_request(const void *msg, size_t n, std::shared_ptr<Client> client_ptr);

  protected:
    void selected(Selector &selector, Selector::Flags flags) noexcept override ;

    void release() noexcept override ;

    size_t receive_message(const void *buf, size_t n) override;

  };

}

#endif //ENGINE_UPLINK_HPP
