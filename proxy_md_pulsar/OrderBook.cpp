
#include "OrderBook.hpp"

using namespace md;
using namespace proxy;

unsigned int crc32b(unsigned char *message) {
  int i, j;
  unsigned int byte, crc, mask;

  i = 0;
  crc = 0xFFFFFFFF;
  while (message[i] != 0) {
    byte = message[i];            // Get next byte.
    crc = crc ^ byte;
    for (j = 7; j >= 0; j--) {    // Do eight times.
      mask = -(crc & 1);
      crc = (crc >> 1) ^ (0xEDB88320 & mask);
    }
    i = i + 1;
  }
  return ~crc;
}

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
  char str[N]; str[0] = '\0';
  int rc = 0, pos = 0;

  //std::lock_guard ob_guard(ob_mutex);

  auto it_a = asks.rbegin();
  auto it_b = bids.rbegin();
  //while (!((it_a == asks.end()) && (it_b == bids.rend()))) {
  auto top_bid_price = std::numeric_limits<long long>::min();
  auto top_ask_price = std::numeric_limits<long long>::max();
  while (it_a != asks.rend()) {
    //elog.debug() << it_a->second.price << it_a->second.quantity << std::endl;
    if ((it_b != bids.rend()) && (it_a->second.price == it_b->second.price)) {
      rc = snprintf(str + pos, N, "%6lld %6lld %6lld %6lld\n", it_b->second.price, it_b->second.quantity, it_a->second.price, it_a->second.quantity); N -= rc; pos += rc;
      if ((top_ask_price > it_a->second.price) && (it_a->second.quantity > 0))
        top_ask_price = it_a->second.price;
      if ((top_bid_price < it_b->second.price) && (it_b->second.quantity > 0))
        top_bid_price = it_b->second.price;
      it_a++; it_b++;
      if (pos > N-1) {
        elog.error() << "Orderbook too big. Stop printing Orderbook..." << std::endl;
        return;
      }
    }
    else {
      rc = snprintf(str + pos, N, "%6s %6s %6lld %6lld\n", "", "", it_a->second.price, it_a->second.quantity);
      N -= rc;
      pos += rc;
      if ((top_ask_price > it_a->second.price) && (it_a->second.quantity > 0))
        top_ask_price = it_a->second.price;
      it_a++;
      if (pos > N-1) {
        elog.error() << "Orderbook too big. Stop printing Orderbook..." << std::endl;
        return;
      }
    }
  }
  while (it_b != bids.rend()) {
    rc = snprintf(str + pos, N, "%6lld %6lld\n", it_b->second.price, it_b->second.quantity); N -= rc; pos += rc;
    if ((top_bid_price < it_b->second.price) && (it_b->second.quantity > 0))
      top_bid_price = it_b->second.price;
    it_b++;
    if (pos > N-1) {
      elog.error() << "Orderbook too big. Stop printing Orderbook..." << std::endl;
      return;
    }
  }
  str[N-1] = '\0';
  elog.debug() << std::endl << std::setw(6) << std::setfill(' ')
               << "  top_bid_price=" << top_bid_price << " top_ask_price=" << top_ask_price <<  std::endl
               << "  top_bid_px   =" << top_bid_px    << " top_ask_px   =" << top_ask_px    <<  std::endl;
  elog.debug() << std::endl << std::string(str) << std::endl;

  if (top_bid_price >= top_ask_price) {
    elog.error() << "top_bid_price >= top_ask_price" << std::endl;
    exit(-1);
  }

}

void OrderBook::update(const book_item &o) {
  auto side = o.side == book_item::book_side::bid ? "BID" : "ASK";
  elog.debug() << side << " " << o.price << " " << o.quantity << std::endl;

  //std::lock_guard ob_guard(ob_mutex);
  if ((market_id != 0) && (market_id != o.market_id)) {
    //elog.error() << "market_id(" << market_id << ") != current market_id(" << o.market_id << ") Stopping..." << std::endl;
    //return;
  }

  if (o.side == book_item::book_side::bid) {
    bids[o.price] = o;
    if ((o.quantity > 0) && (o.price > top_bid_px))
      top_bid_px = o.price;
    else if ((o.quantity == 0) && (o.price == top_bid_px)) {  // top bid is deleted
      top_bid_px = std::numeric_limits<long long>::min();
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
      top_ask_px = std::numeric_limits<long long>::max();
      for (auto it = asks.begin(); it != asks.end(); it++) {
        if (it->second.quantity > 0) {
          top_ask_px = it->second.price;
          break;
        }
      }
    }
  }

  if (bids.empty()) top_bid_px = std::numeric_limits<long long>::min();
  if (asks.empty()) top_ask_px = std::numeric_limits<long long>::max();

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

  // Generate checksum
  int check_sum_cnt = 25;
  std::stringstream ss; int bid_cnt = 0, ask_cnt = 0;
  auto it_bid = last_valid_bids.rbegin();
  auto it_ask = last_valid_asks.begin();
  while ((it_bid != last_valid_bids.rend()) || (it_ask != last_valid_asks.end())) {
    while (it_bid != last_valid_bids.rend()) {
      if ((it_bid->second.quantity != 0) && (bid_cnt < check_sum_cnt)) {
        ss << it_bid->second.price/(float)factor << ":" << it_bid->second.quantity/(float)factor << ":";
        bid_cnt++;
        break;
      }
      it_bid++;
    }
    while (it_ask != last_valid_asks.end()) {
      if ((it_ask->second.quantity != 0) && (ask_cnt < check_sum_cnt)) {
        ss << it_ask->second.price / (float) factor << ":" << it_ask->second.quantity / (float) factor << ":";
        ask_cnt++;
        break;
      }
      it_ask++;
    }
  }
  auto checksum_str = ss.str().substr(0, ss.str().size()-1);
  uint32_t res32 = crc32b((unsigned char*)checksum_str.data());
  signed_res32 = *(int32_t*)(&res32);

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
    last_sent_bids.insert({it->first, it->second});
    if (++cnt == max_entries) break;
  }

  cnt = 0;
  for (auto it = last_valid_asks.begin(); it != last_valid_asks.end(); it++) {
    //if (it->second.price < top_ask_px) continue;
    if (it->second.quantity == 0) continue;

    if ((market_id != 0) && (market_id != it->second.market_id)) {
      //elog.error() << "market_id(" << market_id << ") != current market_id(" << it->second.market_id << ") Stopping..." << std::endl;
      //break;
    }

    json::Array details;
    details.insert(json::Integer(it->second.price));
    details.insert(json::Integer(it->second.quantity));
    details.insert(json::Integer(0));
    details.insert(json::Integer(0));
    asks_pxLevels.insert(std::move(details));
    last_sent_asks.insert({it->first, it->second});
    if (++cnt == max_entries) break;
  }

  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

  data_item.insert("instrument_id", json::Integer(market_id));
  data_item.insert("bids", std::move(bids_pxLevels));
  data_item.insert("asks", std::move(asks_pxLevels));
  data_item.insert("timestamp", json::String(std::to_string(ms)));
  data_item.insert("checksum", json::Integer(signed_res32));
  data_item.insert("seq_num", json::Integer(seq_num));

  data.insert(std::move(data_item));
  snapshot.insert("data", std::move(data));
  if (elog.info_enabled()) {
    //elog.debug() << "seq_num=" << seq_num << std::endl;
    elog.info() << "snapshot:" << snapshot << std::endl;
    elog.info() << "checksum string:" << checksum_str << std::endl;
  }
  return snapshot;
}

std::pair<bool, json::Object> OrderBook::get_orderbook_diff(//book_map_t &bids,
//json::Object OrderBook::get_orderbook_diff(//book_map_t &bids,
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
    while (it != last_valid_bids.rend()) {
      if (it->second.quantity > 0) {
        top_valid_bid_price = it->second.price;
        break;
      }
      it++;
    }
    auto it2 = last_valid_asks.begin();
    while (it2 != last_valid_asks.end()) {
      if (it2->second.quantity > 0) {
        top_valid_ask_price = it2->second.price;
        break;
      }
      it2++;
    }
  }

  // Previous bids which have been deleted
  for (auto it = last_bids.rbegin(); it != last_bids.rend(); it++) {
    if ((it->second.price > top_valid_bid_price) && (it->second.quantity > 0)) {
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
    if ((it->second.price < top_valid_ask_price) && (it->second.quantity > 0)) {
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

//  // Generate checksum
//  int checksum_cnt = 25, bid_cnt = 0, ask_cnt = 0;
//  std::stringstream ss;
//  auto it_bid = bids_pxLevels->rbegin();
//  auto it_ask = asks_pxLevels->begin();
//  while ((it_bid != bids_pxLevels->rend()) || (it_ask != asks_pxLevels->end())) {
//    if (it_bid != bids_pxLevels->rend()) {
//      if (bid_cnt < checksum_cnt) {
//        ss << it_bid[0]->as_array()->at(0)->as_integer() / (float) factor << ":" << it_bid[0]->as_array()->at(1)->as_integer() / (float) factor << ":";
//        bid_cnt++;
//      }
//      it_bid++;
//    }
//    if (it_ask != asks_pxLevels->end()) {
//      if (ask_cnt < checksum_cnt) {
//        ss << it_ask[0]->as_array()->at(0)->as_integer() / (float) factor << ":" << it_ask[0]->as_array()->at(1)->as_integer() / (float) factor << ":";
//        ask_cnt++;
//      }
//      it_ask++;
//    }
//  }
//  auto checksum_str = ss.str().substr(0, ss.str().size()-1);
//  uint32_t res32 = crc32b((unsigned char*)checksum_str.data());
//  int32_t signed_res32 = *(int32_t*)(&res32);

  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
  bool any_diff = !bids_pxLevels->empty() || !asks_pxLevels->empty();

  data_item.insert("instrument_id", json::Integer(market_id));
  data_item.insert("bids", std::move(bids_pxLevels));
  data_item.insert("asks", std::move(asks_pxLevels));
  data_item.insert("timestamp", json::String(std::to_string(ms)));
  //data_item.insert("timestamp", json::String(currentISO8601TimeUTC()));
  data_item.insert("checksum", json::Integer(signed_res32));
  data_item.insert("seq_num", json::Integer(seq_num));

  data.insert(std::move(data_item));
  delta.insert("data", std::move(data));
  if (elog.info_enabled()) {
    elog.info() << "update  :" << delta << std::endl;
    //elog.info() << "checksum string:" << checksum_str << std::endl;
  }
  return std::make_pair(any_diff, delta);
  //return delta;

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
  // ==== Clear last_valid books ====
  for (auto it = last_valid_bids.begin(); it != last_valid_bids.end(); ) {
    if (it->second.quantity == 0)
      it = last_valid_bids.erase(it);
    else
      ++it;
  }
  for (auto it = last_valid_asks.begin(); it != last_valid_asks.end(); ) {
    if (it->second.quantity == 0)
      it = last_valid_asks.erase(it);
    else
      ++it;
  }
  //last_bids = last_valid_bids;
  //last_asks = last_valid_asks;
  last_bids = last_sent_bids; last_sent_bids.clear();
  last_asks = last_sent_asks; last_sent_asks.clear();

  // ==== Clear most updated books ====
  for (auto it = bids.begin(); it != bids.end(); ) {
    if (it->second.quantity == 0) {
      auto it_last_valid_bid = last_valid_bids.find(it->first);
      if (it_last_valid_bid == last_valid_bids.end()) {
        it = bids.erase(it);
      }
      else
        ++it; continue;
    }
    else
      ++it;
  }
  for (auto it = asks.begin(); it != asks.end(); ) {
    if (it->second.quantity == 0) {
      auto it_last_valid_ask = last_valid_asks.find(it->first);
      if (it_last_valid_ask == last_valid_asks.end()) {
        it = asks.erase(it);
      }
      else
        ++it; continue;
    }
    else
      ++it;
  }
  if (bids.empty()) top_bid_px = std::numeric_limits<long long>::min();
  if (asks.empty()) top_ask_px = std::numeric_limits<long long>::max();

  seq_num++;
}
//
//void OrderBook::clear_unused_bids_asks() {
//  // Suppose get_orderbook_diff(...) has been run
//  // So all clients know the most updated top_bid_px, top_ask_px
//  // So we can now delete unused price levels
//  // For example, last_bids = [ 20@120, 20@119, 20@118, 20@117, 20@116, 20@115, ...]
//  //              last_asks = [ 30@130, 30@131, 30@132, 30@133, 30@134, 30@135, ...]
//  // If (top_bid_px, top_ask_px) moves to (116, 118):
//  //                   bids = [  0@120,  0@119,  0@118,  0@117, 20@116, 20@115, ...]
//  //                   asks = [ 25@118, 25@119, 25@120, 25@121, 25@122, 25@123, 25@124, ...]
//  // then
//  //   1. get_orderbook_diff(...) already told clients to set quantity = 0 for prices 120, 119, 118, 117
//  //   2. we can then remove these prices from bids
//  //std::lock_guard ob_guard(ob_mutex);
//  for (auto it = bids.rbegin(); it != bids.rend(); it++) {
//    if ((it->second.price > top_bid_px) && (it->second.price == 0)) {
//      auto it_last_valid_bid = last_valid_bids.find(it->first);
//      if (it_last_valid_bid != last_valid_bids.end()) {
//        bids.erase(it->first);
//        last_valid_bids.erase(it->first);
//      }
//    }
//  }
//  for (auto it = asks.begin(); it != asks.end(); it++) {
//    if ((it->second.price < top_ask_px) && (it->second.price == 0)) {
//      auto it_last_valid_ask = last_valid_asks.find(it->first);
//      if (it_last_valid_ask != last_valid_asks.end()) {
//        asks.erase(it->first);
//        last_valid_asks.erase(it->first);
//      }
//    }
//  }
//  if (bids.empty()) top_bid_px = std::numeric_limits<long long>::min();
//  if (asks.empty()) top_ask_px = std::numeric_limits<long long>::max();
//
//  last_bids = last_valid_bids;
//  last_asks = last_valid_asks;
//  seq_num++;
//}
//
//
//
//
