#include "Uplink.hpp"
#include "core.h"
#include "common/log.h"
#include "common/json.h"
#include <thread>
#include "proxy_lws_utils.hpp"
#include "Client.hpp"

extern Log elog;

namespace proxy {
  using namespace core;

  void Uplink::enqueue_work(std::function<void(void) /* noexcept */> &&work) {
    if (num_queue == 1)
      this->enqueue_1_queue(std::move(work));
    else
      this->enqueue(std::move(work));
  }

  void Uplink::connect(std::function<void(void)> synchronized) {
    socket = connect_with_retry(host, CORE_PORT);
    socket.fcntl(F_SETFL, socket.fcntl(F_GETFL) | O_NONBLOCK);
    selector_out.add(socket, this, Selector::Flags::NONE);
    selector_in.add(socket, this, Selector::Flags::READABLE);
    {
      struct NegotiateProtocolVersion msg;
      msg.opcode = NegotiateProtocolVersion;
      msg.min_version = 8;
      msg.max_version = PROTOCOL_VERSION;
      this->send_message(&msg, sizeof msg);
    }
    {
      struct SetNotificationMask msg;
      msg.opcode = SetNotificationMask;
      msg.mask = flag<BalanceChanged>() | flag<BalanceAdjusted>() | flag<OrderOpened>() | flag<OrderModified>() | flag<OrdersMatched>() | flag<OrderClosed>() | flag<TickerChanged>() | flag<UserPublicKeyChanged>() | flag<UserTradeVolumeChanged>();
      this->send_message(&msg, sizeof msg);
    }
    this->synchronized = std::move(synchronized);
  }

  uint8_t Uplink::protocol_version() const {
    return Transceiver::protocol_version;
  }

  bool Uplink::send_message(const void *msg, size_t n, bool force) {
    std::lock_guard<std::mutex> lock(send_mutex);
    if (size_t n_buf = send_buf.grem()) {
      if ((send_buf.gptr += socket.write(send_buf.gptr, n_buf)) < send_buf.pptr) {
        if (force) {
          send_buf.compact();
          send_buf.append(msg, n);
          return true;
        }
        return false;
      }
      if (n == 0) {
        socket.flush();
      }
      send_buf.clear();
    }
    if (n > 0) {
      size_t w;
      if ((w = socket.write(msg, n)) < n) {
        send_buf.compact();
        send_buf.append(static_cast<const uint8_t *>(msg) + w, n - w);
        selector_out.modify(socket, this, Selector::Flags::WRITABLE);
      } else {
        socket.flush();
      }
    }
    return true;
  }

  void Uplink::selected(Selector &selector, Selector::Flags flags) noexcept {
    if ((flags & Selector::Flags::READABLE) != Selector::Flags::NONE) {
      this->Transceiver::selected(selector, flags);
    }
    if ((flags & Selector::Flags::WRITABLE) != Selector::Flags::NONE) {
      try {
        if (!this->send_message(nullptr, 0)) {
          selector.modify(socket, this, Selector::Flags::WRITABLE);
        }
        return;
      }
      catch (const std::exception &e) {
        if (elog.error_enabled()) {
          elog.error() << "exception while sending message" << ": " << e.what() << std::endl;
        }
      }
      catch (...) {
        if (elog.error_enabled()) {
          elog.error() << "exception while sending message" << std::endl;
        }
      }
      this->release();
    }
  }

  void Uplink::release() noexcept {
    if (elog.fatal_enabled()) {
      elog.fatal() << "uplink to core failed" << std::endl;
    }
    std::abort();
  }

  size_t Uplink::receive_message(const void *buf, size_t n) {
    switch (*static_cast<const Opcode *>(buf)) {
      case BalanceChanged:
      case BalanceAdjusted: {
        auto &msg = *static_cast<const struct BalanceChanged *>(buf);
        size_t msg_size = msg.opcode == BalanceChanged ? sizeof(struct BalanceChanged) : sizeof(struct BalanceAdjusted);
        if (n < msg_size) {
          return 0;
        }
        if (elog.trace_enabled()) {
          std::thread::id this_id = std::this_thread::get_id();
          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
        }
        else if (elog.debug_enabled()) {
          elog.debug() << " >> " << msg << std::endl;
        }
        this->enqueue_work([msg]() noexcept {
          json::Object notice;
          notice.insert("notice", json::String("BalanceChanged"));
          notice.insert("asset", json::Integer(msg.balance.asset_id));
          notice.insert("balance", json::Integer(msg.balance.balance));
          Client::multicast(msg.user_id, notice);
        });
        return msg_size;
      }
      case OrderOpened: {
        auto &msg = *static_cast<const struct OrderOpened *>(buf);
        if (n < sizeof msg) {
          return 0;
        }
        if (elog.trace_enabled()) {
          std::thread::id this_id = std::this_thread::get_id();
          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
        }
        else if (elog.debug_enabled()) {
          elog.debug() << " >> " << msg << std::endl;
        }

        this->enqueue_work([msg]() noexcept {
          get_book(msg.order.order_info.asset_pair, true)->order_opened(msg);
        });
        return sizeof msg;
      }
      case OrderModified: {
        auto &msg = *static_cast<const struct OrderModified *>(buf);
        if (n < sizeof msg) {
          return 0;
        }
        if (elog.trace_enabled()) {
          std::thread::id this_id = std::this_thread::get_id();
          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
        }
        else if (elog.debug_enabled()) {
          elog.debug() << " >> " << msg << std::endl;
        }

        this->enqueue_work([msg]() noexcept {
          get_book(msg.order.order_info.asset_pair, true)->order_modified(msg);
        });
        return sizeof msg;
      }
      case OrdersMatched: {
        auto &msg = *static_cast<const struct OrdersMatched *>(buf);
        if (n < sizeof msg) {
          return 0;
        }
        if (elog.trace_enabled()) {
          std::thread::id this_id = std::this_thread::get_id();
          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
        }
        else if (elog.debug_enabled()) {
          elog.debug() << " >> " << msg << std::endl;
        }

        this->enqueue_work([msg]() noexcept {
          get_book(msg.asset_pair, true)->orders_matched(msg);
        });
        return sizeof msg;
      }
      case OrderClosed: {
        auto &msg = *static_cast<const struct OrderClosed *>(buf);
        if (n < sizeof msg) {
          return 0;
        }
        if (elog.trace_enabled()) {
          std::thread::id this_id = std::this_thread::get_id();
          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
        }
        else if (elog.debug_enabled()) {
          elog.debug() << " >> " << msg << std::endl;
        }

        this->enqueue_work([msg]() noexcept {
          Client::clear_order(msg.user_id, msg.order.order_id);
          get_book(msg.order.order_info.asset_pair, true)->order_closed(msg);
        });
        return sizeof msg;
      }
      case TickerChanged: {
        auto &msg = *static_cast<const struct TickerChanged *>(buf);
        if (n < sizeof msg) {
          return 0;
        }
        if (elog.trace_enabled()) {
          std::thread::id this_id = std::this_thread::get_id();
          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
        }
        else if (elog.debug_enabled()) {
          elog.debug() << " >> " << msg << std::endl;
        }

        this->enqueue_work([msg]() noexcept {
          get_book(msg.asset_pair, true)->ticker_changed(msg);
        });
        return sizeof msg;
      }
      case UserPublicKeyChanged: {
        auto &msg = *static_cast<const struct UserPublicKeyChanged *>(buf);
        if (n < sizeof msg) {
          return 0;
        }
        this->enqueue_work([msg]() noexcept {
          if (elog.trace_enabled()) {
            std::thread::id this_id = std::this_thread::get_id();
            elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
          }
          else if (elog.debug_enabled()) {
            elog.debug() << " >> " << msg << std::endl;
          }

          {
            std::lock_guard<std::shared_mutex> users_wrlock(users_rwlock);
            if (msg.user_id >= users.size()) {
              users.resize(msg.user_id + 1);
            }
            users[msg.user_id].public_key = msg.public_key;
          }
          Client::disconnect_user(msg.user_id);
        });
        return sizeof msg;
      }
      case UserTradeVolumeChanged: {
        auto &msg = *static_cast<const struct UserTradeVolumeChanged *>(buf);
        if (n < sizeof msg) {
          return 0;
        }
        if (elog.trace_enabled()) {
          std::thread::id this_id = std::this_thread::get_id();
          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
        }
        else if (elog.debug_enabled()) {
          elog.debug() << " >> " << msg << std::endl;
        }

        this->enqueue_work([msg]() noexcept {
          json::Object notice;
          notice.insert("notice", json::String("TradeVolumeChanged"));
          notice.insert("asset", json::Integer(msg.asset_id));
          notice.insert("volume", json::Integer(msg.volume));
          Client::multicast(msg.user_id, notice);
        });
        return sizeof msg;
      }
      case Success:
      case NotFound:
      case Exists:
      case OutOfSequence:
      case InsufficientFunds:
      case TooMany:
      case TooFast:
      case InvalidArgument:
      case NotAllowed:
        /*
        case OutOfBounds:
        case UnknownError: */
      {
        std::unique_lock<std::mutex> callback_queue_lock(callback_queue_mutex);
        if (callback_queue.empty()) {
          throw std::ios_base::failure("received spurious response from core");
        }
        auto &client_ptr = callback_queue.front();
        callback_queue_lock.unlock();
        if ((n = client_ptr->receive_response(buf, n)) != 0) {
          callback_queue_lock.lock();
          callback_queue.pop();
        }
        return n;
      }
      case Synchronized:
        this->enqueue_work([this]() noexcept {
          if (elog.debug_enabled()) {
            elog.debug() << ">> " << Synchronized << std::endl;
          }
          if (synchronized) {
            synchronized(), synchronized = nullptr;
          }
        });
        return sizeof(Opcode);
      case NegotiateProtocolVersion: {
        auto ret = this->Transceiver::receive_message(*static_cast<const struct NegotiateProtocolVersion *>(buf), n);
        if (ret && this->protocol_version() < 4 && synchronized) {
          if (elog.warn_enabled()) {
            elog.warn() << "upstream server does not indicate completion of state synchronization" << std::endl;
          }
          synchronized(), synchronized = nullptr;
        }
        return ret;
      }
      default:
        throw std::ios_base::failure("received illegal opcode");
    }
  }

  bool Uplink::do_request(const void *msg, size_t n, std::shared_ptr<Client> client_ptr) {
    std::lock_guard<std::mutex> callback_queue_lock(callback_queue_mutex);
    if (this->send_message(msg, n)) {
      callback_queue.push(std::move(client_ptr));
      return true;
    }
    return false;
  }
}