#include "state.h"

#include <algorithm>
#include <cassert>

#include "common/dns.h"


namespace state {


std::array<std::unique_lock<std::mutex>, 2> lock_users(id_t user1_id, User &user1, id_t user2_id, User &user2) {
	std::array<std::unique_lock<std::mutex>, 2> user_locks;
	if (user1_id < user2_id) {
		user_locks[0] = std::unique_lock<std::mutex>(user1.mutex);
		user_locks[1] = std::unique_lock<std::mutex>(user2.mutex);
	}
	else {
		user_locks[0] = std::unique_lock<std::mutex>(user2.mutex);
		if (user1_id != user2_id) {
			user_locks[1] = std::unique_lock<std::mutex>(user1.mutex);
		}
	}
	return user_locks;
}


void Book::order_opened_locked(User &user, const struct OrderOpened &msg) {
	if (msg.order.order_info.quantity == 0) {
		return;
	}
	std::map<id_t, std::list<OrderRecord>::iterator>::iterator user_orders_by_id_itr;
	bool inserted;
	std::tie(user_orders_by_id_itr, inserted) = user.orders_by_id.try_emplace(msg.order.order_id);
	if (!inserted) {
		return;
	}
	auto &order_itr = user_orders_by_id_itr->second;
	if (msg.order.order_info.quantity > 0) {
		auto book_bids_itr = bids.try_emplace(msg.order.order_info.price).first;
		order_itr = book_bids_itr->second.emplace(book_bids_itr->second.end(), msg);
		order_itr->book_bids_itr = book_bids_itr;
	}
	else {
		auto book_asks_itr = asks.try_emplace(msg.order.order_info.price).first;
		order_itr = book_asks_itr->second.emplace(book_asks_itr->second.end(), msg);
		order_itr->book_asks_itr = book_asks_itr;
	}
	order_itr->user_orders_by_id_itr = user_orders_by_id_itr;
	if (order_itr->tonce) {
		order_itr->user_orders_by_tonce_itr = user.orders_by_tonce.emplace(order_itr->tonce, order_itr);
	}
}

void Book::order_modified_locked(User &user, const struct OrderModified &msg) {
	auto user_orders_by_id_itr = user.orders_by_id.find(msg.order.order_id);
	if (user_orders_by_id_itr != user.orders_by_id.end()) {
		this->order_modified_locked(user_orders_by_id_itr->second, msg);
	}
}

void Book::order_modified_locked(std::list<OrderRecord>::iterator order_itr, const struct OrderModified &msg) {
	assert(msg.order.order_info.tonce == order_itr->tonce);
	assert(msg.order.order_info.asset_pair == order_itr->asset_pair);
	assert(msg.order.order_info.price != 0);
	if (order_itr->quantity > 0) {
		assert(msg.order.order_info.quantity > 0);
		auto book_bids_itr = order_itr->book_bids_itr;
		if (msg.order.order_info.price != order_itr->price) {
			auto new_bids_itr = bids.try_emplace(msg.order.order_info.price).first;
			new_bids_itr->second.splice(new_bids_itr->second.end(), book_bids_itr->second, order_itr);
			if (book_bids_itr->second.empty()) {
				bids.erase(book_bids_itr);
			}
			order_itr->book_bids_itr = new_bids_itr;
		}
		else if (msg.order.order_info.quantity > order_itr->quantity) {
			book_bids_itr->second.splice(book_bids_itr->second.end(), book_bids_itr->second, order_itr);
		}
	}
	else if (order_itr->quantity < 0) {
		assert(msg.order.order_info.quantity < 0);
		auto book_asks_itr = order_itr->book_asks_itr;
		if (msg.order.order_info.price != order_itr->price) {
			auto new_asks_itr = asks.try_emplace(msg.order.order_info.price).first;
			new_asks_itr->second.splice(new_asks_itr->second.end(), book_asks_itr->second, order_itr);
			if (book_asks_itr->second.empty()) {
				asks.erase(book_asks_itr);
			}
			order_itr->book_asks_itr = new_asks_itr;
		}
		else if (msg.order.order_info.quantity < order_itr->quantity) {
			book_asks_itr->second.splice(book_asks_itr->second.end(), book_asks_itr->second, order_itr);
		}
	}
	else {
		throw std::logic_error("order has zero quantity");
	}
	order_itr->quantity = msg.order.order_info.quantity;
	order_itr->price = msg.order.order_info.price;
	order_itr->time = msg.order.order_info.time;
}

void Book::orders_matched_locked(User &bid_user, User &ask_user, const struct OrdersMatched &msg) {
	if (~msg.bid_order_id) {
		auto user_orders_by_id_itr = bid_user.orders_by_id.find(msg.bid_order_id);
		if (user_orders_by_id_itr != bid_user.orders_by_id.end()) {
			auto bid_itr = user_orders_by_id_itr->second;
			if (msg.bid_remaining_quantity == 0) {
				this->order_closed_locked(bid_user, bid_itr);
			}
			else {
				bid_itr->quantity = msg.bid_remaining_quantity;
			}
		}
	}
	if (~msg.ask_order_id) {
		auto user_orders_by_id_itr = ask_user.orders_by_id.find(msg.ask_order_id);
		if (user_orders_by_id_itr != ask_user.orders_by_id.end()) {
			auto ask_itr = user_orders_by_id_itr->second;
			if (msg.ask_remaining_quantity == 0) {
				this->order_closed_locked(ask_user, ask_itr);
			}
			else {
				ask_itr->quantity = -msg.ask_remaining_quantity;
			}
		}
	}
}

void Book::order_closed_locked(User &user, const struct OrderClosed &msg) {
	auto user_orders_by_id_itr = user.orders_by_id.find(msg.order.order_id);
	if (user_orders_by_id_itr != user.orders_by_id.end()) {
		return this->order_closed_locked(user, user_orders_by_id_itr->second);
	}
}

void Book::order_closed_locked(User &user, std::list<OrderRecord>::const_iterator order_itr) {
	user.orders_by_id.erase(order_itr->user_orders_by_id_itr);
	if (order_itr->tonce) {
		user.orders_by_tonce.erase(order_itr->user_orders_by_tonce_itr);
	}
	if (order_itr->quantity > 0) {
		auto book_bids_itr = order_itr->book_bids_itr;
		book_bids_itr->second.erase(order_itr);
		if (book_bids_itr->second.empty()) {
			bids.erase(book_bids_itr);
		}
	}
	else if (order_itr->quantity < 0) {
		auto book_asks_itr = order_itr->book_asks_itr;
		book_asks_itr->second.erase(order_itr);
		if (book_asks_itr->second.empty()) {
			asks.erase(book_asks_itr);
		}
	}
	else {
		throw std::logic_error("order has zero quantity");
	}
}

void Book::ticker_changed_locked(const struct TickerChanged &msg) {
	last = msg.last;
	low_24h = msg.low_24h;
	high_24h = msg.high_24h;
	volume_24h = msg.volume_24h;
}

struct TickerChanged Book::make_ticker_notice_locked(const asset_pair_t &asset_pair, timestamp_t time) const {
	struct TickerChanged notice;
	notice.opcode = TickerChanged;
	notice.asset_pair = asset_pair;
	notice.last = last;
	notice.bid = bids.empty() ? 0 : bids.begin()->first;
	notice.ask = asks.empty() ? 0 : asks.begin()->first;
	notice.low_24h = low_24h;
	notice.high_24h = high_24h;
	notice.volume_24h = volume_24h;
	notice.time = time;
	return notice;
}


} // namespace state
