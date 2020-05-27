#include "Book.hpp"
#include "Client.hpp"

namespace proxy {
  void Book::order_opened(const struct OrderOpened &msg) {
    json::Object::map_t::iterator notice_tonce_itr, notice_fee_itr;
    auto notice = order_to_json(msg.order, true, &notice_tonce_itr, &notice_fee_itr);
    notice.insert("notice", json::String("OrderOpened"));
    Client::multicast(msg.user_id, notice);
    notice->erase(notice_tonce_itr);
    notice->erase(notice_fee_itr);

    std::lock_guard<std::mutex> lock(mutex);
    Client::multicast(orders_clients, notice, msg.user_id);
    if (msg.order.order_info.quantity > 0) {
      auto &bids = this->bids[msg.order.order_info.price];
      auto &pair = orders.try_emplace(orders.end(), msg.order.order_id, true, bids.end())->second;
      if (pair.second == bids.end()) {
        pair.second = bids.emplace(bids.end(), msg.order);
      }
    } else if (msg.order.order_info.quantity < 0) {
      auto &asks = this->asks[msg.order.order_info.price];
      auto &pair = orders.try_emplace(orders.end(), msg.order.order_id, false, asks.end())->second;
      if (pair.second == asks.end()) {
        pair.second = asks.emplace(asks.end(), msg.order);
      }
    }
  }

  void Book::order_modified(const struct OrderModified &msg) {
    json::Object::map_t::iterator notice_tonce_itr;
    json::Object notice = order_to_json(msg.order, true, &notice_tonce_itr);
    notice.insert("notice", json::String("OrderModified"));
    auto user_predicate = [](id_t, uint8_t api_version) { return api_version >= 1; };
    //Client::multicast(msg.user_id, CachingFormatter<json::Value, decltype(user_predicate)>(notice, user_predicate));
    Client::multicast(msg.user_id, notice);
    notice->erase(notice_tonce_itr);
    auto others_predicate = [&msg](id_t user_id, uint8_t api_version) { return user_id != msg.user_id && api_version >= 1; };

    std::lock_guard<std::mutex> lock(mutex);
    //Client::multicast(orders_clients, CachingFormatter<json::Value, decltype(others_predicate)>(notice, others_predicate));
    Client::multicast(orders_clients, notice, msg.user_id);
    auto orders_itr = orders.find(msg.order.order_id);
    if (orders_itr != orders.end()) {
      auto order_itr = orders_itr->second.second;
      {
        auto user_predicate = [](id_t, uint8_t api_version) { return api_version < 1; };
        auto others_predicate = [&msg](id_t user_id, uint8_t api_version) { return user_id != msg.user_id && api_version < 1; };
        {
          json::Object::map_t::iterator notice_tonce_itr;
          json::Object notice = order_to_json(*order_itr, false, &notice_tonce_itr);
          notice.insert("notice", json::String("OrderClosed"));
          //Client::multicast(msg.user_id, CachingFormatter<json::Value, decltype(user_predicate)>(notice, user_predicate));
          notice->erase(notice_tonce_itr);
          //Client::multicast(orders_clients, CachingFormatter<json::Value, decltype(others_predicate)>(notice, others_predicate));
        }
        {
          json::Object::map_t::iterator notice_tonce_itr;
          json::Object notice = order_to_json(msg.order, true, &notice_tonce_itr);
          notice.insert("notice", json::String("OrderOpened"));
          //Client::multicast(msg.user_id, CachingFormatter<json::Value, decltype(user_predicate)>(notice, user_predicate));
          notice->erase(notice_tonce_itr);
          //Client::multicast(orders_clients, CachingFormatter<json::Value, decltype(others_predicate)>(notice, others_predicate));
        }
      }
      assert(msg.order.order_info.tonce == order_itr->order_info.tonce);
      assert(msg.order.order_info.asset_pair == order_itr->order_info.asset_pair);
      assert(msg.order.order_info.quantity != 0);
      assert(msg.order.order_info.price != 0);
      if (orders_itr->second.first) {
        if (msg.order.order_info.price != order_itr->order_info.price) {
          auto old_bids_itr = bids.find(order_itr->order_info.price), new_bids_itr = bids.try_emplace(msg.order.order_info.price).first;
          new_bids_itr->second.splice(new_bids_itr->second.end(), old_bids_itr->second, order_itr);
          if (old_bids_itr->second.empty()) {
            bids.erase(old_bids_itr);
          }
        } else if (msg.order.order_info.quantity > order_itr->order_info.quantity) {
          auto &bids = this->bids.at(order_itr->order_info.price);
          bids.splice(bids.end(), bids, order_itr);
        }
      } else {
        if (msg.order.order_info.price != order_itr->order_info.price) {
          auto old_asks_itr = asks.find(order_itr->order_info.price), new_asks_itr = asks.try_emplace(msg.order.order_info.price).first;
          new_asks_itr->second.splice(new_asks_itr->second.end(), old_asks_itr->second, order_itr);
          if (old_asks_itr->second.empty()) {
            asks.erase(old_asks_itr);
          }
        } else if (msg.order.order_info.quantity < order_itr->order_info.quantity) {
          auto &asks = this->asks.at(order_itr->order_info.price);
          asks.splice(asks.end(), asks, order_itr);
        }
      }
      order_itr->order_info.quantity = msg.order.order_info.quantity;
      order_itr->order_info.price = msg.order.order_info.price;
    }
  }

  void Book::orders_matched(const struct OrdersMatched &msg) {

    auto add_bid_info = [](const json::Object &notice, const struct OrdersMatched &msg) {
      json::Object result(notice);
      msg.bid_tonce ? result.insert("bid_tonce", json::Integer(msg.bid_tonce)) : result.insert("bid_tonce", nullptr);
      result.insert("bid_base_fee", json::Integer(msg.bid_base_fee));
      result.insert("bid_counter_fee", json::Integer(msg.bid_counter_fee));
      result.insert("taker", json::Boolean(msg.bid_flags.taker));
      return result;
    };
    auto add_ask_info = [](const json::Object &notice, const struct OrdersMatched &msg) {
      json::Object result(notice);
      msg.ask_tonce ? result.insert("ask_tonce", json::Integer(msg.ask_tonce)) : result.insert("ask_tonce", nullptr);
      result.insert("ask_base_fee", json::Integer(msg.ask_base_fee));
      result.insert("ask_counter_fee", json::Integer(msg.ask_counter_fee));
      result.insert("taker", json::Boolean(msg.ask_flags.taker));
      return result;
    };
    auto insert_order_cause = [](const json::Object &notice, const OrderOrigin &origin) {
      json::Object result(notice);
      if (OrderOrigin::DAEMON == origin) {
        result.insert("cause", json::String("liquidation"));
      }
      return result;
    };
    json::Object notice;

    notice.insert("notice", json::String("OrdersMatched"));
    if (~msg.bid_order_id) {
      notice.insert("bid", json::Integer(msg.bid_order_id));
    }
    if (~msg.ask_order_id) {
      notice.insert("ask", json::Integer(msg.ask_order_id));
    }
    notice.insert("base", json::Integer(msg.asset_pair.first));
    notice.insert("counter", json::Integer(msg.asset_pair.second));
    notice.insert("quantity", json::Integer(msg.quantity));
    notice.insert("price", json::Integer(msg.price));
    notice.insert("total", json::Integer(msg.total));
    if (msg.bid_flags.taker ^ msg.ask_flags.taker) {
      if (msg.bid_flags.taker) {
        notice.insert("taker_side", json::String("bid"));
      } else {
        notice.insert("taker_side", json::String("ask"));
      }
    }
    if (~msg.bid_remaining_quantity) {
      notice.insert("bid_rem", json::Integer(msg.bid_remaining_quantity));
    }
    if (~msg.ask_remaining_quantity) {
      notice.insert("ask_rem", json::Integer(msg.ask_remaining_quantity));
    }
    notice.insert("time", json::Integer(std::chrono::duration_cast<std::chrono::microseconds>(msg.time.time_since_epoch()).count()));
    if (msg.ask_user_id == msg.bid_user_id) {
      json::Object result = add_ask_info(add_bid_info(notice, msg), msg);
      Client::multicast(msg.bid_user_id, result);
    } else {
      Client::multicast(msg.bid_user_id,
                        insert_order_cause(add_bid_info(notice, msg),
                                           static_cast<OrderOrigin>(msg.bid_flags.origin)));
      Client::multicast(msg.ask_user_id,
                        insert_order_cause(add_ask_info(notice, msg),
                                           static_cast<OrderOrigin>(msg.ask_flags.origin)));
    }

    std::lock_guard<std::mutex> lock(mutex);
    Client::multicast(orders_clients, notice, msg.bid_user_id, msg.ask_user_id);
    auto orders_itr = orders.find(msg.bid_order_id);
    if (orders_itr != orders.end()) {
      orders_itr->second.second->order_info.quantity = msg.bid_remaining_quantity;
    }
    orders_itr = orders.find(msg.ask_order_id);
    if (orders_itr != orders.end()) {
      orders_itr->second.second->order_info.quantity = -msg.ask_remaining_quantity;
    }
  }

  void Book::order_closed(const struct OrderClosed &msg) {
    json::Object::map_t::iterator notice_tonce_itr;
    auto notice = order_to_json(msg.order, false, &notice_tonce_itr);
    notice.insert("notice", json::String("OrderClosed"));
    notice.insert("time_closed", json::Integer(std::chrono::duration_cast<std::chrono::microseconds>(msg.time_closed.time_since_epoch()).count()));
    Client::multicast(msg.user_id, notice);
    notice->erase(notice_tonce_itr);

    std::lock_guard<std::mutex> lock(mutex);
    Client::multicast(orders_clients, notice, msg.user_id);
    auto orders_itr = orders.find(msg.order.order_id);
    if (orders_itr != orders.end()) {
      if (orders_itr->second.first) {
        auto bids_itr = bids.find(msg.order.order_info.price);
        if (bids_itr != bids.end() && (bids_itr->second.erase(orders_itr->second.second), bids_itr->second.empty())) {
          bids.erase(bids_itr);
        }
      } else {
        auto asks_itr = asks.find(msg.order.order_info.price);
        if (asks_itr != asks.end() && (asks_itr->second.erase(orders_itr->second.second), asks_itr->second.empty())) {
          asks.erase(asks_itr);
        }
      }
      orders.erase(orders_itr);
    }
  }

  void Book::ticker_changed(const struct TickerChanged &msg) {
    auto notice = ticker_to_json(msg, &ticker);
    std::lock_guard<std::mutex> lock(mutex);
    if (!notice->empty()) {
      notice.insert("notice", json::String("TickerChanged"));
      notice.insert("time", json::Integer(msg.asset_pair.first));
      notice.insert("base", json::Integer(msg.asset_pair.first));
      notice.insert("counter", json::Integer(msg.asset_pair.second));
      Client::multicast(ticker_clients, notice);
    }
    ticker = msg;
  }

  void Book::remove_ticker_client(std::list<Client *>::iterator itr) {
    std::lock_guard<std::mutex> lock(mutex);
    ticker_clients.erase(itr);
  }

  std::pair<std::list<Client *>::iterator, json::Object> Book::add_ticker_client(Client *client_ptr) {
    std::lock_guard<std::mutex> lock(mutex);
    return {ticker_clients.insert(ticker_clients.end(), client_ptr), ticker_to_json(ticker)};
  }

  void Book::remove_orders_client(std::list<Client *>::iterator itr) {
    std::lock_guard<std::mutex> lock(mutex);
    orders_clients.erase(itr);
  }

  std::pair<std::list<Client *>::iterator, json::Object> Book::add_orders_client(Client *client_ptr) {
    std::lock_guard<std::mutex> lock(mutex);
    json::Object response;
    json::Array response_orders;
    response_orders->reserve(2000);
    size_t n = 0;
    for (auto bids_itr = bids.begin(); bids_itr != bids.end() && n < 1000; ++bids_itr) {
      for (auto bid_itr = bids_itr->second.begin(); bid_itr != bids_itr->second.end() && n < 1000; ++bid_itr, ++n) {
        json::Object order;
        order.insert("id", json::Integer(bid_itr->order_id));
        order.insert("quantity", json::Integer(bid_itr->order_info.quantity));
        order.insert("price", json::Integer(bid_itr->order_info.price));
        order.insert("time", json::Integer(std::chrono::duration_cast<std::chrono::microseconds>(bid_itr->order_info.time.time_since_epoch()).count()));
        response_orders.insert(std::move(order));
      }
    }
    n = 0;
    for (auto asks_itr = asks.begin(); asks_itr != asks.end() && n < 1000; ++asks_itr) {
      for (auto ask_itr = asks_itr->second.begin(); ask_itr != asks_itr->second.end() && n < 1000; ++ask_itr, ++n) {
        json::Object order;
        order.insert("id", json::Integer(ask_itr->order_id));
        order.insert("quantity", json::Integer(ask_itr->order_info.quantity));
        order.insert("price", json::Integer(ask_itr->order_info.price));
        order.insert("time", json::Integer(std::chrono::duration_cast<std::chrono::microseconds>(ask_itr->order_info.time.time_since_epoch()).count()));
        response_orders.insert(std::move(order));
      }
    }
    response.insert("orders", std::move(response_orders));
    return {orders_clients.insert(orders_clients.end(), client_ptr), std::move(response)};
  }
/*
  std::pair<uint64_t, uint64_t> Book::estimate_market_order_from_total(int64_t total) {
    std::lock_guard<std::mutex> lock(mutex);
    uint64_t traded_quantity = 0, traded_total = 0, last_quantity = 0;
    if (total < 0) {
      total = -total;
      for (auto &bids : this->bids) {
        for (auto &bid : bids.second) {
          uint64_t bid_total = ::muladddiv(static_cast<uint64_t>(bid.order_info.quantity), bid.order_info.price, PRICE_SCALE - 1, PRICE_SCALE).first;
          if (static_cast<uint64_t>(total) > bid_total) {
            traded_quantity += bid.order_info.quantity;
            traded_total += bid_total;
            total -= bid_total;
          } else {
            last_quantity = ::muldiv(static_cast<uint64_t>(total), PRICE_SCALE, bid.order_info.price).first;
            traded_quantity += last_quantity;
            traded_total += ::muldiv(static_cast<uint64_t>(last_quantity), bid.order_info.price, PRICE_SCALE).first;
            goto end;
          }
        }
      }
    } else if (total > 0) {
      for (auto &asks : this->asks) {
        for (auto &ask : asks.second) {
          uint64_t ask_total = ::muladddiv(static_cast<uint64_t>(-ask.order_info.quantity), ask.order_info.price, PRICE_SCALE - 1, PRICE_SCALE).first;
          if (static_cast<uint64_t>(total) > ask_total) {
            traded_quantity += -ask.order_info.quantity;
            traded_total += ask_total;
            total -= ask_total;
          } else {
            last_quantity = ::muldiv(static_cast<uint64_t>(total), PRICE_SCALE, ask.order_info.price).first;
            traded_quantity += last_quantity;
            traded_total += ::muldiv(static_cast<uint64_t>(last_quantity), ask.order_info.price, PRICE_SCALE).first;
            goto end;
          }
        }
      }
    }
    end:
    return {traded_quantity, traded_total};
  } */
/*
  std::pair<uint64_t, uint64_t> Book::estimate_market_order_from_quantity(int64_t quantity) {
    std::lock_guard<std::mutex> lock(mutex);
    std::uniform_int_distribution<uint64_t> price_dist(0, PRICE_SCALE - 1);
    uint64_t traded_quantity = 0, traded_total = 0;
    if (quantity < 0) {
      quantity = -quantity;
      for (auto &bids : this->bids) {
        for (auto &bid : bids.second) {
          uint64_t trade_quantity = std::min<uint64_t>(bid.order_info.quantity, quantity);
          traded_quantity += trade_quantity;
          traded_total += ::muladddiv(trade_quantity, bid.order_info.price, price_dist(rand), PRICE_SCALE).first;
          if ((quantity -= trade_quantity) == 0) {
            goto end;
          }
        }
      }
    } else if (quantity > 0) {
      for (auto &asks : this->asks) {
        for (auto &ask : asks.second) {
          uint64_t trade_quantity = std::min<uint64_t>(-ask.order_info.quantity, quantity);
          traded_quantity += trade_quantity;
          traded_total += ::muladddiv(trade_quantity, ask.order_info.price, price_dist(rand), PRICE_SCALE).first;
          if ((quantity -= trade_quantity) == 0) {
            goto end;
          }
        }
      }
    }
    end:
    return {traded_quantity, traded_total};
  }*/

  Book *get_book(const asset_pair_t &pair, bool create ) {
    {
      std::shared_lock<std::shared_mutex> books_rdlock(books_rwlock);
      auto book_itr = books.find(pair);
      if (book_itr != books.end()) {
        return &book_itr->second;
      }

      if (!create) {
        return nullptr;
      }
    }
    std::lock_guard<std::shared_mutex> books_wrlock(books_rwlock);
    return &books[pair];
  }
}