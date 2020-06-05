#include "Uplink.hpp"
#include "core.h"
#include "common/json.h"
#include <thread>
#include <ctime>
#include "proxy_lws_utils.hpp"
#include "Client.hpp"
#include <matching/order.hpp>
#include <md/book_item.hpp>

std::unordered_map<unsigned long long, unsigned long long> client_to_engine_id_map;





json::Object handle_order(const matching::order& o)
{
    std::string type = "";
    std::string side = "";
    std::string status = "";
    std::string time_condition = "";
    std::string action = "";
    std::string matched_type = "";
    if (matching::order::order_type::LIMITED == o.type)
    {
      type = "LIMITED";
    }
    else
    {
      type = "MARKET";
    }
    if (matching::order::order_side::BUY == o.side)
    {
      side = "BUY";
    }
    else if (matching::order::order_side::SELL == o.side)
    {
      side = "SELL";
    }
    else if (matching::order::order_side::BUY_STOP == o.side)
    {
      side = "BUY_STOP";
    }
    else if (matching::order::order_side::SELL_STOP == o.side)
    {
      side = "SELL_STOP";
    }
    else if (matching::order::order_side::BUY_SELL_STOP == o.side)
    {
      side = "BUY_SELL_STOP";
    }
    else
    {
      side = "SELL_BUY_STOP";
    }
    if (o.order_state == matching::order::order_status_type::OPEN)
    {
      status = "OPEN";
    }
    else if (o.order_state == matching::order::order_status_type::PARTIAL_FILL)
    {
      status = "PARTIAL_FILL";
    }
    else if (o.order_state == matching::order::order_status_type::FILLED)
    {
      status = "FILLED";
    }
    else if (o.order_state == matching::order::order_status_type::CANCELED_BY_USER)
    {
      status = "CANCELED_BY_USER";
    }
    else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED)
    {
      status = "CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED";
    }
    else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MARKET_ORDER_NOTHING_MATCH)
    {
      status = "CANCELED_BY_MARKET_ORDER_NOTHING_MATCH";
    }
    else if (o.order_state == matching::order::order_status_type::CANCELED_ALL_BY_IOC)
    {
      status = "CANCELED_ALL_BY_IOC";
    }
    else if (o.order_state == matching::order::order_status_type::CANCELED_PARTIAL_BY_IOC)
    {
      status = "CANCELED_PARTIAL_BY_IOC";
    }
    else if (o.order_state == matching::order::order_status_type::CANCELED_BY_FOK)
    {
      status = "CANCELED_BY_FOK";
    }
    else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MAKER_ONLY)
    {
      status = "CANCELED_BY_MAKER_ONLY";
    }
    else if (o.order_state == matching::order::order_status_type::REJECT_CANCEL_ORDER_ID_NOT_FOUND)
    {
      status = "REJECT_CANCEL_ORDER_ID_NOT_FOUND";
    }
    else if (o.order_state == matching::order::order_status_type::REJECT_AMEND_ORDER_ID_NOT_FOUND)
    {
      status = "REJECT_AMEND_ORDER_ID_NOT_FOUND";
    }
    else if (o.order_state == matching::order::order_status_type::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY)
    {
      status = "REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY";
    }
    else if (o.order_state == matching::order::order_status_type::REJECT_BUY_STOP_TRIGGER_LARGE_THAN_STOP_LIMITED)
    {
      status = "REJECT_BUY_STOP_TRIGGER_LESS_THAN_STOP_LIMITED";
    }
    else if (o.order_state == matching::order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED)
    {
      status = "REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED";
    }
    else if (o.order_state == matching::order::order_status_type::REJECT_UNKNOW_ORDER_ACTION)
    {
      status = "REJECT_UNKNOW_ORDER_ACTION";
    }
    else if (o.order_state == matching::order::order_status_type::REJECT_QUANTITY_ZERO)
    {
      status = "REJECT_QUANTITY_ZERO";
    }
    else
    {
      status = "REJECT_LIMITE_ORDER_WITH_MARKET_PRICE";
    }
    if (matching::order::order_time_condition::GTC == o.time_condition)
    {
      time_condition = "GTC";
    }
    else if (matching::order::order_time_condition::IOC == o.time_condition)
    {
      time_condition = "IOC";
    }
    else if (matching::order::order_time_condition::FOK == o.time_condition)
    {
      time_condition = "FOK";
    }
    else if (matching::order::order_time_condition::MAKER_ONLY == o.time_condition)
    {
      time_condition = "MAKER_ONLY";
    }
    else
    {
      time_condition = "MAKER_ONLY_REPRICE";
    }
    if (matching::order::order_action_type::NEW == o.order_action)
    {
      action = "NEW";
    }
    else if (matching::order::order_action_type::CANCEL == o.order_action)
    {
      action = "CANCEL";
    }
    else
    {
      action = "AMEND";
    }
    if (matching::order::order_matched_type::TAKER == o.matched_type)
    {
      matched_type = "TAKER";
    }
    else
    {
      matched_type = "MAKER";
    }
  client_to_engine_id_map[o.client_order_id] = o.order_id;
  elog.debug()
      << "action:" << action
      << ",side:" << side
      << ",time_condition:" << time_condition
      << ",order_id:" << o.order_id
      << ",client_order_id:" << o.client_order_id
      << ",quantity:" << o.quantity
      << ",display_quantity:" << o.display_quantity
      << ",remain_quantity:" << o.remain_quantity
      << ",price:" << o.price
      << ",buy_stop_trigger_price:" << o.buy_stop_trigger_price
      << ",buy_stop_limited_price:" << o.buy_stop_limited_price
      << ",sell_stop_trigger_price:" << o.sell_stop_trigger_price
      << ",sell_stop_limited_price:" << o.sell_stop_limited_price
      << ",last_match_price:" << o.last_match_price
      << ",last_match_quantity:" << o.last_match_quantity
      << ",last_matched_order_id:" << o.last_matched_order_id
      << ",last_matched_order_id2:" << o.last_matched_order_id2
      << ",matched_id:" << o.matched_id
      << ",status:" << status
      << ",matched_type:" << matched_type
      << std::endl;

  json::Object response;
  response.insert("action", json::String(action));
  response.insert("side", json::String(side));
  response.insert("time_condition", json::String(time_condition));
  response.insert("order_id", json::Integer(o.order_id));
  response.insert("client_order_id", json::Integer(o.client_order_id));
  response.insert("quantity", json::Integer(o.quantity));
  response.insert("display_quantity", json::Integer(o.display_quantity));
  response.insert("remain_quantity", json::Integer(o.remain_quantity));
  response.insert("price", json::Integer(o.price));
  response.insert("buy_stop_trigger_price", json::Integer(o.buy_stop_trigger_price));
  response.insert("buy_stop_limited_price", json::Integer(o.buy_stop_limited_price));
  response.insert("sell_stop_trigger_price", json::Integer(o.sell_stop_trigger_price));
  response.insert("sell_stop_limited_price", json::Integer(o.sell_stop_limited_price));
  response.insert("last_match_price", json::Integer(o.last_match_price));
  response.insert("last_match_quantity", json::Integer(o.last_match_quantity));
  response.insert("last_matched_order_id", json::Integer(o.last_matched_order_id));
  response.insert("last_matched_order_id2", json::Integer(o.last_matched_order_id2));
  response.insert("matched_id", json::Integer(o.matched_id));
  response.insert("status", json::String(status));
  response.insert("matched_type", json::String(matched_type));

  //response.insert("time",
  //    json::Integer(std::chrono::duration_cast<std::chrono::microseconds>(reply.time.time_since_epoch()).count()));

  return response;
}


namespace proxy {
  using namespace core;
  std::chrono::steady_clock::duration Uplink::broadcast_interval;
/*
  json::Object Uplink::handle_book_item(const md::book_item& o) {
    if (o.side == md::book_item::book_side::bid) {
      if (o.quantity == 0)
        bids.erase(o.price);
      else
        bids[o.price] = o;
    }
    else if (o.side == md::book_item::book_side::ask) {
      if (o.quantity == 0)
        asks.erase(o.price);
      else
        asks[o.price] = o;
    }


    std::string side = "";
    if (o.side == md::book_item::book_side::bid)
      side = "BID";
    else if (o.side == md::book_item::book_side::bid)
      side = "ASK";
    elog.debug()
        << "px:" << o.price
        << ",qty:" << o.quantity
        << ",side:" << o.side
        << ",market_id:" << o.market_id
        << std::endl;
    json::Object response;
    response.insert("px", json::Integer(o.price));
    response.insert("qty", json::Integer(o.quantity));
    response.insert("side", json::String(side));
    response.insert("market_id", json::Integer(o.market_id));

    return response;
  }
*/
  void Uplink::enqueue_work(std::function<void(void) /* noexcept */> &&work) {
    if (num_queue == 1)
      this->enqueue_1_queue(std::move(work));
    else
      this->enqueue(std::move(work));
  }

  void Uplink::connect(std::function<void(void)> synchronized) {
    socket = connect_with_retry(host, port);
    socket.fcntl(F_SETFL, socket.fcntl(F_GETFL) | O_NONBLOCK);
    selector_out.add(socket, this, Selector::Flags::NONE);
    selector_in.add(socket, this, Selector::Flags::READABLE);
    /*
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
    } */
    this->synchronized = std::move(synchronized);
  }

  void Uplink::disconnect() {
    if (elog.debug_enabled()) elog.debug() << "Disconnecting engine..." << std::endl;
    selector_out.remove(socket);
    selector_in.remove(socket);
    socket.close();
  }

  void Uplink::reconnect() {
    disconnect();
    connect(this->synchronized);
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
    {
      size_t msg_size = sizeof(md::book_item);
      if (n < msg_size) return 0;

      auto &reply = (*static_cast<const md::book_item *>(buf));
      {
        std::lock_guard ob_guard(ob.ob_mutex);
        ob.update(reply);
        ob.print_bids_asks(ob.last_valid_bids, ob.last_valid_asks);
      }
      //json::Object response = handle_book_item(reply);
      //Client::multicast(Client::all_clients, response);
      //std::stringstream ss;
      //ss << response;
      //pub_sock.send(zmq::const_buffer(ss.str().c_str(),  ss.str().size()), zmq::send_flags::none);

//      std::unique_lock<std::mutex> callback_queue_lock(callback_queue_mutex);
//      if (callback_queue.empty()) {
//        throw std::ios_base::failure("received spurious response from core");
//      }
//      auto &client_ptr = callback_queue.front();
//      callback_queue_lock.unlock();
//      if ((n = client_ptr->receive_response(buf, n)) != 0) {
//        callback_queue_lock.lock();
//        callback_queue.pop();
//      }
      return msg_size;
    }

    size_t msg_size = sizeof(matching::order);
    if (n < msg_size) return 0;
//    auto callback = [this, tag, transient](const void *buf, size_t n) -> size_t {
//        auto &reply = (*static_cast<const matching::order *>(buf));
//
//        json::Object response = handle_order(reply);
//        if (tag)
//          response.insert("tag", json::Integer(tag));
//        this->send_message(response);
//    };

    auto &msg = *static_cast<const matching::order *>(buf);
    this->enqueue_work([msg]() noexcept {
      json::Object notice = handle_order(msg);
      Client::multicast(0, notice);
    });

//    handle_order(msg);
    return msg_size;
    {

//    switch (*static_cast<const Opcode *>(buf)) {
//      case BalanceChanged:
//      case BalanceAdjusted: {
//        auto &msg = *static_cast<const struct BalanceChanged *>(buf);
//        size_t msg_size = msg.opcode == BalanceChanged ? sizeof(struct BalanceChanged) : sizeof(struct BalanceAdjusted);
//        if (n < msg_size) {
//          return 0;
//        }
//        if (elog.trace_enabled()) {
//          std::thread::id this_id = std::this_thread::get_id();
//          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
//        }
//        else if (elog.debug_enabled()) {
//          elog.debug() << " >> " << msg << std::endl;
//        }
//        this->enqueue_work([msg]() noexcept {
//          json::Object notice;
//          notice.insert("notice", json::String("BalanceChanged"));
//          notice.insert("asset", json::Integer(msg.balance.asset_id));
//          notice.insert("balance", json::Integer(msg.balance.balance));
//          Client::multicast(msg.user_id, notice);
//        });
//        return msg_size;
//      }
//      case OrderOpened: {
//        auto &msg = *static_cast<const struct OrderOpened *>(buf);
//        if (n < sizeof msg) {
//          return 0;
//        }
//        if (elog.trace_enabled()) {
//          std::thread::id this_id = std::this_thread::get_id();
//          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
//        }
//        else if (elog.debug_enabled()) {
//          elog.debug() << " >> " << msg << std::endl;
//        }
//
//        this->enqueue_work([msg]() noexcept {
//          get_book(msg.order.order_info.asset_pair, true)->order_opened(msg);
//        });
//        return sizeof msg;
//      }
//      case OrderModified: {
//        auto &msg = *static_cast<const struct OrderModified *>(buf);
//        if (n < sizeof msg) {
//          return 0;
//        }
//        if (elog.trace_enabled()) {
//          std::thread::id this_id = std::this_thread::get_id();
//          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
//        }
//        else if (elog.debug_enabled()) {
//          elog.debug() << " >> " << msg << std::endl;
//        }
//
//        this->enqueue_work([msg]() noexcept {
//          get_book(msg.order.order_info.asset_pair, true)->order_modified(msg);
//        });
//        return sizeof msg;
//      }
//      case OrdersMatched: {
//        auto &msg = *static_cast<const struct OrdersMatched *>(buf);
//        if (n < sizeof msg) {
//          return 0;
//        }
//        if (elog.trace_enabled()) {
//          std::thread::id this_id = std::this_thread::get_id();
//          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
//        }
//        else if (elog.debug_enabled()) {
//          elog.debug() << " >> " << msg << std::endl;
//        }
//
//        this->enqueue_work([msg]() noexcept {
//          get_book(msg.asset_pair, true)->orders_matched(msg);
//        });
//        return sizeof msg;
//      }
//      case OrderClosed: {
//        auto &msg = *static_cast<const struct OrderClosed *>(buf);
//        if (n < sizeof msg) {
//          return 0;
//        }
//        if (elog.trace_enabled()) {
//          std::thread::id this_id = std::this_thread::get_id();
//          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
//        }
//        else if (elog.debug_enabled()) {
//          elog.debug() << " >> " << msg << std::endl;
//        }
//
//        this->enqueue_work([msg]() noexcept {
//          Client::clear_order(msg.user_id, msg.order.order_id);
//          get_book(msg.order.order_info.asset_pair, true)->order_closed(msg);
//        });
//        return sizeof msg;
//      }
//      case TickerChanged: {
//        auto &msg = *static_cast<const struct TickerChanged *>(buf);
//        if (n < sizeof msg) {
//          return 0;
//        }
//        if (elog.trace_enabled()) {
//          std::thread::id this_id = std::this_thread::get_id();
//          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
//        }
//        else if (elog.debug_enabled()) {
//          elog.debug() << " >> " << msg << std::endl;
//        }
//
//        this->enqueue_work([msg]() noexcept {
//          get_book(msg.asset_pair, true)->ticker_changed(msg);
//        });
//        return sizeof msg;
//      }
//      case UserPublicKeyChanged: {
//        auto &msg = *static_cast<const struct UserPublicKeyChanged *>(buf);
//        if (n < sizeof msg) {
//          return 0;
//        }
//        this->enqueue_work([msg]() noexcept {
//          if (elog.trace_enabled()) {
//            std::thread::id this_id = std::this_thread::get_id();
//            elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
//          }
//          else if (elog.debug_enabled()) {
//            elog.debug() << " >> " << msg << std::endl;
//          }
//
//          {
//            std::lock_guard<std::shared_mutex> users_wrlock(users_rwlock);
//            if (msg.user_id >= users.size()) {
//              users.resize(msg.user_id + 1);
//            }
//            users[msg.user_id].public_key = msg.public_key;
//          }
//          Client::disconnect_user(msg.user_id);
//        });
//        return sizeof msg;
//      }
//      case UserTradeVolumeChanged: {
//        auto &msg = *static_cast<const struct UserTradeVolumeChanged *>(buf);
//        if (n < sizeof msg) {
//          return 0;
//        }
//        if (elog.trace_enabled()) {
//          std::thread::id this_id = std::this_thread::get_id();
//          elog.trace() << "Thread(" << this_id << ") >> " << msg << std::endl;
//        }
//        else if (elog.debug_enabled()) {
//          elog.debug() << " >> " << msg << std::endl;
//        }
//
//        this->enqueue_work([msg]() noexcept {
//          json::Object notice;
//          notice.insert("notice", json::String("TradeVolumeChanged"));
//          notice.insert("asset", json::Integer(msg.asset_id));
//          notice.insert("volume", json::Integer(msg.volume));
//          Client::multicast(msg.user_id, notice);
//        });
//        return sizeof msg;
//      }
//      case Success:
//      case NotFound:
//      case Exists:
//      case OutOfSequence:
//      case InsufficientFunds:
//      case TooMany:
//      case TooFast:
//      case InvalidArgument:
//      case NotAllowed:
//        /*
//        case OutOfBounds:
//        case UnknownError: */
//      {
//        std::unique_lock<std::mutex> callback_queue_lock(callback_queue_mutex);
//        if (callback_queue.empty()) {
//          throw std::ios_base::failure("received spurious response from core");
//        }
//        auto &client_ptr = callback_queue.front();
//        callback_queue_lock.unlock();
//        if ((n = client_ptr->receive_response(buf, n)) != 0) {
//          callback_queue_lock.lock();
//          callback_queue.pop();
//        }
//        return n;
//      }
//      case Synchronized:
//        this->enqueue_work([this]() noexcept {
//          if (elog.debug_enabled()) {
//            elog.debug() << ">> " << Synchronized << std::endl;
//          }
//          if (synchronized) {
//            synchronized(), synchronized = nullptr;
//          }
//        });
//        return sizeof(Opcode);
//      case NegotiateProtocolVersion: {
//        auto ret = this->Transceiver::receive_message(*static_cast<const struct NegotiateProtocolVersion *>(buf), n);
//        if (ret && this->protocol_version() < 4 && synchronized) {
//          if (elog.warn_enabled()) {
//            elog.warn() << "upstream server does not indicate completion of state synchronization" << std::endl;
//          }
//          synchronized(), synchronized = nullptr;
//        }
//        return ret;
//      }
//      default:
//        throw std::ios_base::failure("received illegal opcode");
//    }
//
    }
}

  bool Uplink::do_request(const void *msg, size_t n, std::shared_ptr<proxy::Client> client_ptr) {
    std::lock_guard<std::mutex> callback_queue_lock(callback_queue_mutex);
    if (this->send_message(msg, n)) {
      callback_queue.push(std::move(client_ptr));
      return true;
    }
    return false;
  }

  struct cmp {
    bool operator()(const Uplink::book_map_t::value_type &a,
                    const Uplink::book_map_t::value_type &b) const {
      return
          ( (a.first < b.first) ||
            ((a.first == b.first) && (a.second.quantity < a.second.quantity)) );
    }
  };

/*
  json::Object Uplink::get_orderbook_diff(book_map_t &bids,      book_map_t &asks,
                                          book_map_t &last_bids, book_map_t &last_asks) {


    // need to lock?
    json::Array bids_pxLevels;
    json::Array asks_pxLevels;
    json::Array book;
    json::Array data;
    json::Object data_item;
    json::Object response ;
    response.insert("table", json::String("futures/depth"));
    response.insert("action", json::String("partial"));

    unsigned long long last_market_id = 0;
    int cnt = 0;
    long long min_bid = std::numeric_limits<long long>::min();
    long long top_bid = min_bid;
    for (auto it = bids.rbegin(); it != bids.rend(); it++) {
      if ((last_market_id != 0) && (last_market_id != it->second.market_id)) {
        elog.error() << "last_market_id(" << last_market_id << ") != current market_id(" << it->second.market_id << ") Stopping..." << std::endl;
        break;
      }
      last_market_id = it->second.market_id;

      json::Array details;
      details.insert(json::Integer(it->second.price));
      details.insert(json::Integer(it->second.quantity));
      details.insert(json::Integer(0));
      details.insert(json::Integer(0));
      bids_pxLevels.insert(std::move(details));
      if ((top_bid == min_bid) && (it->second.quantity > 0))
        top_bid = it->second.price;
      if (++cnt == 400) break;
    }

    last_market_id = 0;
    cnt = 0;
    long long max_ask = std::numeric_limits<long long>::max();
    long long top_ask = max_ask;
    for (auto it = asks.begin(); it != asks.end(); it++) {
      if ((last_market_id != 0) && (last_market_id != it->second.market_id)) {
        elog.error() << "last_market_id(" << last_market_id << ") != current market_id(" << it->second.market_id << ") Stopping..." << std::endl;
        break;
      }
      last_market_id = it->second.market_id;

      json::Array details;
      details.insert(json::Integer(it->second.price));
      details.insert(json::Integer(it->second.quantity));
      details.insert(json::Integer(0));
      details.insert(json::Integer(0));
      asks_pxLevels.insert(std::move(details));
      if ((top_ask == max_ask) && (it->second.quantity > 0))
        top_ask = it->second.price;
      if (++cnt == 400) break;
    }
//    json::Array details;; details.insert(json::Integer(0)); details.insert(json::Integer(1)); details.insert(json::Integer(2));
//    bids_pxLevels.insert(std::move(details)); bids_pxLevels.insert(std::move(details)); bids_pxLevels.insert(std::move(details));
//    asks_pxLevels.insert(std::move(details)); asks_pxLevels.insert(std::move(details)); asks_pxLevels.insert(std::move(details));

    data_item.insert("instrument_id", json::Integer(last_market_id));
    data_item.insert("bids", std::move(bids_pxLevels));
    data_item.insert("asks", std::move(asks_pxLevels));
    data_item.insert("timestamp", json::String(currentISO8601TimeUTC()));
    data_item.insert("checksum", json::Integer(0));

    data.insert(std::move(data_item));
    response.insert("data", std::move(data));
    if (elog.debug_enabled()) {
      elog.debug() << "response:  " << response << std::endl;
    }
    return response;
  }
*/

  void Uplink::schedule_broadcast() noexcept {
    scheduler.call_at(next_broadcast_time,
        [this]() noexcept {
//      if (elog.debug_enabled()) {
//        elog.debug() << "schedule_broadcast  " << std::endl;
//      }

//      json::Object response ;
//      response.insert("Testing", json::String("TestValue"));
//      Client::multicast(Client::all_clients, response);
      json::Object ob_snapshot, ob_diff;
      {
        std::lock_guard ob_guard(ob.ob_mutex);
        ob_snapshot = this->ob.get_orderbook_snapshot();
        ob_diff     = this->ob.get_orderbook_diff();
        this->ob.clear_unused_bids_asks();
        //this->ob.print_bids_asks(ob.last_valid_bids, ob.last_valid_asks);
      }
      Client::multicast_orderbook(Client::all_clients, ob_snapshot, ob_diff);
      pulsar_broadcast(ob_snapshot, prd_snapshot);
      pulsar_broadcast(ob_diff    , prd_diff);

      this->next_broadcast_time = std::chrono::steady_clock::now() + broadcast_interval;
      this->schedule_broadcast();

    });
  }

  void Uplink::pulsar_broadcast(const json::Object &jo, Producer &prd) {
    std::stringstream ss;
    ss << jo;
    parser.builder_.Clear();
    if (!parser.Parse(ss.str().c_str())) {
      elog.debug() << "flatbuffers parser failed with error : " << parser.error_ << std::endl;
      return;
    }
    uint8_t *buf = parser.builder_.GetBufferPointer();
    uint32_t sz  = parser.builder_.GetSize();
    Message msg = MessageBuilder().setContent(buf, sz).build();
    Result res = prd.send(msg);
    elog.info() << "Message sent: " << res << std::endl;
    //zmq_sock.send(zmq::const_buffer(buf, sz), zmq::send_flags::none);
  }

  void Uplink::reschedule_broadcast() noexcept {
    next_broadcast_time = std::chrono::steady_clock::now() + broadcast_interval;
  }
}