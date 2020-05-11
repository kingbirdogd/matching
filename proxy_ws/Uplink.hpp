#ifndef ENGINE_UPLINK_HPP
#define ENGINE_UPLINK_HPP

#include <matching/order.hpp>
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

  public:
    Uplink(const char host[], Selector &selector_in, Selector &selector_out, uint8_t num_queue) : host(host), selector_in(selector_in),
                                                                                                  selector_out(selector_out), num_queue(num_queue) {}

    void enqueue_work(std::function<void(void) /* noexcept */> &&work);

    void connect(std::function<void(void)> synchronized = nullptr);

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
