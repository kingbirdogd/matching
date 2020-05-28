#ifndef ENGINE_BOOK_HPP
#define ENGINE_BOOK_HPP

#include <map>
#include <list>
//#include "Client.hpp"
#include "core.h"
#include <random>
#include "common/muldiv.h"
#include "common/json.h"
#include <shared_mutex>

namespace proxy {
  using namespace core;
  using core::id_t ;

  class Client;

  static json::Object order_to_json(const Order &order, bool include_time = true, json::Object::map_t::iterator *tonce_itr_ptr = nullptr, json::Object::map_t::iterator *fee_itr_ptr = nullptr) {
    json::Object obj;
    obj.insert("id", json::Integer(order.order_id));
    auto tonce_itr = (order.order_info.tonce ? obj.insert("tonce", json::Integer(order.order_info.tonce)) : obj.insert("tonce", nullptr)).first;
    if (tonce_itr_ptr) {
      *tonce_itr_ptr = tonce_itr;
    }
    obj.insert("base", json::Integer(order.order_info.asset_pair.first));
    obj.insert("counter", json::Integer(order.order_info.asset_pair.second));
    obj.insert("quantity", json::Integer(order.order_info.quantity));
    obj.insert("price", json::Integer(order.order_info.price));
    if (include_time) {
      obj.insert("time", json::Integer(std::chrono::duration_cast<std::chrono::microseconds>(order.order_info.time.time_since_epoch()).count()));
    }
    auto fee_itr = obj.insert("fee", json::String(order.order_info.flags.fee_external ? "ext" : "int")).first;
    if (fee_itr_ptr) {
      *fee_itr_ptr = fee_itr;
    }
    return obj;
  }

  static json::Object ticker_to_json(const struct TickerChanged &ticker, const struct TickerChanged *base = nullptr) {
    json::Object notice;
    if (!base || ticker.last != base->last) {
      if (ticker.last) {
        notice.insert("last", json::Integer(ticker.last));
      } else {
        notice.insert("last", nullptr);
      }
    }
    if (!base || ticker.bid != base->bid) {
      if (ticker.bid) {
        notice.insert("bid", json::Integer(ticker.bid));
      } else {
        notice.insert("bid", nullptr);
      }
    }
    if (!base || ticker.ask != base->ask) {
      if (ticker.ask) {
        notice.insert("ask", json::Integer(ticker.ask));
      } else {
        notice.insert("ask", nullptr);
      }
    }
    if (!base || ticker.low_24h != base->low_24h) {
      if (ticker.low_24h) {
        notice.insert("low", json::Integer(ticker.low_24h));
      } else {
        notice.insert("low", nullptr);
      }
    }
    if (!base || ticker.high_24h != base->high_24h) {
      if (ticker.high_24h) {
        notice.insert("high", json::Integer(ticker.high_24h));
      } else {
        notice.insert("high", nullptr);
      }
    }
    if (!base || ticker.volume_24h != base->volume_24h) {
      notice.insert("volume", json::Integer(ticker.volume_24h));
    }
    return notice;
  }

  class Book {

  private:
    std::mutex mutex;
    std::map<uint64_t, std::list<Order>, std::greater<uint64_t>> bids;
    std::map<uint64_t, std::list<Order>> asks;
    std::map<id_t, std::pair<bool, std::list<Order>::iterator>> orders;
    std::list<Client *> orders_clients, ticker_clients;
    struct TickerChanged ticker;
    std::minstd_rand rand;
    uint16_t min_order_qty{1};

  public:
    void set_min_order_qty(const uint16_t qty) { min_order_qty = qty; }

    uint16_t get_min_order_qty() { return min_order_qty; }

    std::pair<uint64_t, uint64_t> estimate_market_order_from_quantity(int64_t quantity);

    std::pair<uint64_t, uint64_t> estimate_market_order_from_total(int64_t total);

    std::pair<std::list<Client *>::iterator, json::Object> add_orders_client(Client *client_ptr) ;

    void remove_orders_client(std::list<Client *>::iterator itr);

    std::pair<std::list<Client *>::iterator, json::Object> add_ticker_client(Client *client_ptr);

    void remove_ticker_client(std::list<Client *>::iterator itr);

    void order_opened(const struct OrderOpened &msg);

    void order_modified(const struct OrderModified &msg);

    void orders_matched(const struct OrdersMatched &msg);

    void order_closed(const struct OrderClosed &msg);

    void ticker_changed(const struct TickerChanged &msg);

  };

  extern std::shared_mutex books_rwlock;
  extern std::map<asset_pair_t, Book> books;

  Book *get_book(const asset_pair_t &pair, bool create = false) ;

}
#endif //ENGINE_BOOK_HPP
