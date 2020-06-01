
#include "OrderBook.hpp"

using namespace md;
using namespace proxy;

std::string currentISO8601TimeUTC() {
  auto now = std::chrono::system_clock::now();
  auto itt = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
  std::ostringstream ss;
  //ss << std::put_time(gmtime(&itt), "%FT%TZ");
  ss << std::put_time(gmtime(&itt), "%FT%T.")
     << std::setfill('0') << std::setw(3) << ms << "Z";
  return ss.str();
}

void OrderBook::print_bids_asks(book_map_t &bids, book_map_t &asks) {
  int N = 4096;
  char str[N];
  int rc = 0, pos = 0;

  std::lock_guard ob_guard(ob_mutex);

  auto it_a = asks.rbegin();
  auto it_b = bids.rbegin();
  //while (!((it_a == asks.end()) && (it_b == bids.rend()))) {
  auto top_bid_price = std::numeric_limits<long long>::min();
  auto top_ask_price = std::numeric_limits<long long>::max();
  while (it_a != asks.rend()) {
    //elog.debug() << it_a->second.price << it_a->second.quantity << std::endl;
    if ((it_b != bids.rend()) && (it_a->second.price == it_b->second.price)) {
      rc = snprintf(str + pos, N, "%6d %6d %6d %6d\n", it_b->second.price, it_b->second.quantity, it_a->second.price, it_a->second.quantity); N -= rc; pos += rc;
      if ((top_ask_price > it_a->second.price) && (it_a->second.quantity > 0))
        top_ask_price = it_a->second.price;
      if ((top_bid_price < it_b->second.price) && (it_b->second.quantity > 0))
        top_bid_price = it_b->second.price;
      it_a++; it_b++;
    }
    else {
      rc = snprintf(str + pos, N, "%6s %6s %6d %6d\n", "", "", it_a->second.price, it_a->second.quantity);
      N -= rc;
      pos += rc;
      if ((top_ask_price > it_a->second.price) && (it_a->second.quantity > 0))
        top_ask_price = it_a->second.price;
      it_a++;
    }
  }
  while (it_b != bids.rend()) {
    rc = snprintf(str + pos, N, "%6d %6d\n", it_b->second.price, it_b->second.quantity); N -= rc; pos += rc;
    if ((top_bid_price < it_b->second.price) && (it_b->second.quantity > 0))
      top_bid_price = it_b->second.price;
    it_b++;
  }

  elog.debug() << std::endl << std::setw(6) << std::setfill(' ')
               << "top_bid_price=" << top_bid_price << " top_ask_price=" << top_ask_price <<  std::endl
               << "top_bid_px   =" << top_bid_px    << " top_ask_px   =" << top_ask_px    <<  std::endl
               << std::string(str) << std::endl;

  if (top_bid_price >= top_ask_price) {
    elog.debug() << "ERROR: top_bid_price >= top_ask_price" << std::endl;
    exit(-1);
  }

}

void OrderBook::update(const book_item &o) {
  auto side = o.side == book_item::book_side::bid ? "BID" : "ASK";
  elog.debug() << side << " " << o.price << " " << o.quantity << std::endl;

  std::lock_guard ob_guard(ob_mutex);
  if ((market_id != 0) && (market_id != o.market_id)) {
    elog.error() << "market_id(" << market_id << ") != current market_id(" << o.market_id << ") Stopping..." << std::endl;
    return;
  }

  if (o.side == book_item::book_side::bid) {
    bids[o.price] = o;
    if ((o.quantity > 0) && (o.price > top_bid_px))
      top_bid_px = o.price;
    else if ((o.quantity == 0) && (o.price == top_bid_px)) {  // top bid is deleted
      for (auto it = bids.rbegin(); it != bids.rend(); it++) {
        if (it->second.quantity > 0) {
          top_bid_px = it->second.price;
          break;
        }
      }
    }
  }
  else if (o.side == book_item::book_side::ask) {
    asks[o.price] = o;
    if ((o.quantity > 0) && (o.price < top_ask_px))
      top_ask_px = o.price;
    else if ((o.quantity == 0) && (o.price == top_ask_px)) {  // top ask is deleted
      for (auto it = asks.begin(); it != asks.end(); it++) {
        if (it->second.quantity > 0) {
          top_ask_px = it->second.price;
          break;
        }
      }
    }
  }

  if (top_bid_px < top_ask_px) {
    last_valid_bids = bids;
    last_valid_asks = asks;
  }
}

json::Object OrderBook::get_orderbook_snapshot(//OrderBook::book_map_t &bids,
                                               //OrderBook::book_map_t &asks,
                                               uint32_t max_entries) {
  //std::lock_guard ob_guard(ob_mutex);
  json::Array bids_pxLevels, asks_pxLevels, book, data;
  json::Object data_item, snapshot;
  snapshot.insert("table", json::String("futures/depth"));
  snapshot.insert("action", json::String("partial"));

  int cnt = 0;
  for (auto it = last_valid_bids.rbegin(); it != last_valid_bids.rend(); it++) {
    //if (it->second.price > top_bid_px) continue;
    if (it->second.quantity == 0) continue;

    json::Array details;
    details.insert(json::Integer(it->second.price));
    details.insert(json::Integer(it->second.quantity));
    details.insert(json::Integer(0));
    details.insert(json::Integer(0));
    bids_pxLevels.insert(std::move(details));
    if (++cnt == max_entries) break;
  }

  cnt = 0;
  for (auto it = last_valid_asks.begin(); it != last_valid_asks.end(); it++) {
    //if (it->second.price < top_ask_px) continue;
    if (it->second.quantity == 0) continue;

    if ((market_id != 0) && (market_id != it->second.market_id)) {
      elog.error() << "market_id(" << market_id << ") != current market_id(" << it->second.market_id << ") Stopping..." << std::endl;
      break;
    }

    json::Array details;
    details.insert(json::Integer(it->second.price));
    details.insert(json::Integer(it->second.quantity));
    details.insert(json::Integer(0));
    details.insert(json::Integer(0));
    asks_pxLevels.insert(std::move(details));
    if (++cnt == max_entries) break;
  }

  data_item.insert("instrument_id", json::Integer(market_id));
  data_item.insert("bids", std::move(bids_pxLevels));
  data_item.insert("asks", std::move(asks_pxLevels));
  data_item.insert("timestamp", json::String(currentISO8601TimeUTC()));
  data_item.insert("checksum", json::Integer(0));
  data_item.insert("seq_num", json::Integer(seq_num));

  data.insert(std::move(data_item));
  snapshot.insert("data", std::move(data));
  if (elog.debug_enabled()) {
    //elog.debug() << "seq_num=" << seq_num << std::endl;
    elog.debug() << "snapshot:" << snapshot << std::endl;
  }
  return snapshot;
}

json::Object OrderBook::get_orderbook_diff(//book_map_t &bids,
                                           //book_map_t &asks,
                                           //book_map_t &last_bids,
                                           //book_map_t &last_asks,
                                           uint32_t max_entries) {
  //std::lock_guard ob_guard(ob_mutex);
  json::Array bids_pxLevels, asks_pxLevels, book, data;
  json::Object data_item, delta;
  delta.insert("table", json::String("futures/depth"));
  delta.insert("action", json::String("update"));

  auto top_valid_bid_price = std::numeric_limits<long long>::min();
  auto top_valid_ask_price = std::numeric_limits<long long>::max();
  {
    auto it = last_valid_bids.rbegin();
    while (it++ != last_valid_bids.rend()) top_valid_bid_price = it->second.price;
  }
  {
    auto it = last_valid_asks.begin();
    while (it++ != last_valid_asks.end()) top_valid_ask_price = it->second.price;
  }

  // Previous bids which have been deleted
  for (auto it = last_bids.rbegin(); it != last_bids.rend(); it++) {
    if (it->second.price > top_valid_bid_price) {
      json::Array details;
      details.insert(json::Integer(it->second.price));
      details.insert(json::Integer(0));
      details.insert(json::Integer(0));
      details.insert(json::Integer(0));
      bids_pxLevels.insert(std::move(details));
    }
    else
      break;
  }
  int cnt = 0;
  for (auto it = last_valid_bids.rbegin(); it != last_valid_bids.rend(); it++) {
    if (it->second.price <= top_valid_bid_price) {
      auto last_it = last_bids.find(it->first);
      if ((last_it == last_bids.end()) || (last_it->second != it->second)) {
        json::Array details;
        details.insert(json::Integer(it->second.price));
        details.insert(json::Integer(it->second.quantity));
        details.insert(json::Integer(0));
        details.insert(json::Integer(0));
        bids_pxLevels.insert(std::move(details));
      }
    }
    if (++cnt == max_entries) break;
  }

  // Previous asks which have been deleted
  for (auto it = last_asks.begin(); it != last_asks.end(); it++) {
    if (it->second.price < top_valid_ask_price) {
      json::Array details;
      details.insert(json::Integer(it->second.price));
      details.insert(json::Integer(0));
      details.insert(json::Integer(0));
      details.insert(json::Integer(0));
      asks_pxLevels.insert(std::move(details));
    }
    else
      break;
  }
  cnt = 0;
  for (auto it = last_valid_asks.begin(); it != last_valid_asks.end(); it++) {
    if (it->second.price >= top_valid_ask_price) {
      auto last_it = last_asks.find(it->first);
      if ((last_it == last_asks.end()) || (last_it->second != it->second)) {
        json::Array details;
        details.insert(json::Integer(it->second.price));
        details.insert(json::Integer(it->second.quantity));
        details.insert(json::Integer(0));
        details.insert(json::Integer(0));
        asks_pxLevels.insert(std::move(details));
      }
    }
    if (++cnt == max_entries) break;
  }

  data_item.insert("instrument_id", json::Integer(market_id));
  data_item.insert("bids", std::move(bids_pxLevels));
  data_item.insert("asks", std::move(asks_pxLevels));
  data_item.insert("timestamp", json::String(currentISO8601TimeUTC()));
  data_item.insert("checksum", json::Integer(0));
  data_item.insert("seq_num", json::Integer(seq_num));

  data.insert(std::move(data_item));
  delta.insert("data", std::move(data));
  if (elog.debug_enabled()) {
    elog.debug() << "update  :" << delta << std::endl;
  }
  return delta;

}

void OrderBook::clear_unused_bids_asks() {
  // Suppose get_orderbook_diff(...) has been run
  // So all clients know the most updated top_bid_px, top_ask_px
  // So we can now delete unused price levels
  // For example, last_bids = [ 20@120, 20@119, 20@118, 20@117, 20@116, 20@115, ...]
  //              last_asks = [ 30@130, 30@131, 30@132, 30@133, 30@134, 30@135, ...]
  // If (top_bid_px, top_ask_px) moves to (116, 118):
  //                   bids = [  0@120,  0@119,  0@118,  0@117, 20@116, 20@115, ...]
  //                   asks = [ 25@118, 25@119, 25@120, 25@121, 25@122, 25@123, 25@124, ...]
  // then
  //   1. get_orderbook_diff(...) already told clients to set quantity = 0 for prices 120, 119, 118, 117
  //   2. we can then remove these prices from bids
  //std::lock_guard ob_guard(ob_mutex);
  for (auto it = bids.rbegin(); it != bids.rend(); it++) {
    if ((it->second.price > top_bid_px) && (it->second.price == 0)) {
      auto it_last_valid_bid = last_valid_bids.find(it->first);
      if (it_last_valid_bid != last_valid_bids.end()) {
        bids.erase(it->first);
        last_valid_bids.erase(it->first);
      }
    }
  }
  for (auto it = asks.begin(); it != asks.end(); it++) {
    if ((it->second.price < top_ask_px) && (it->second.price == 0)) {
      auto it_last_valid_ask = last_valid_asks.find(it->first);
      if (it_last_valid_ask != last_valid_asks.end()) {
        asks.erase(it->first);
        last_valid_asks.erase(it->first);
      }
    }
  }
  if (bids.empty()) top_bid_px = std::numeric_limits<long long>::min();
  if (asks.empty()) top_ask_px = std::numeric_limits<long long>::max();

  last_bids = last_valid_bids;
  last_asks = last_valid_asks;
  seq_num++;
}



