#ifndef ENGINE_ORDERBOOK_HPP
#define ENGINE_ORDERBOOK_HPP

#include <md/book_item.hpp>
#include <map>
#include <common/json.h>
#include <chrono>
#include <common/log.h>
#include <iomanip>
#include <mutex>

extern Log elog;
using namespace md;
using namespace std::chrono;

namespace proxy {
  class OrderBook {
  public:
    using book_map_t = std::map<long long, md::book_item>;
    OrderBook() : top_bid_px(std::numeric_limits<long long>::min()),
                  top_ask_px(std::numeric_limits<long long>::max()),
                  seq_num(duration_cast<seconds>(system_clock::now().time_since_epoch()).count() * 1e9),
                  market_id(0),
                  factor(100000000)
    {
      elog.debug() << "seq_num=" << seq_num << std::endl;
    }
    void set_market_id(unsigned long long mid) { market_id = mid; }
    void update(const book_item& bi);
    //json::Object get_orderbook_snapshot(book_map_t &bids, book_map_t &asks, uint32_t max_enteries=400);
    json::Object get_orderbook_snapshot(uint32_t max_enteries=400);
    std::pair<bool, json::Object> get_orderbook_diff(uint32_t max_enteries=400);
//    json::Object get_orderbook_diff(book_map_t &bids     , book_map_t &asks,
//                                    book_map_t &last_bids, book_map_t &last_asks,
//                                    uint32_t max_enteries=400);
    //void clear_unused_bids_asks(book_map_t &bids, book_map_t &asks, long long top_bid, long long top_ask);
    void clear_unused_bids_asks();
    void print_bids_asks(book_map_t &bids, book_map_t &asks);
    book_map_t last_valid_bids, last_valid_asks;
    std::mutex ob_mutex;
  private:
    book_map_t bids, asks, last_bids, last_asks;
    long long top_bid_px, top_ask_px;
    unsigned long long seq_num;
    unsigned long long market_id;
    unsigned long long factor;
  };



}
#endif //ENGINE_ORDERBOOK_HPP
