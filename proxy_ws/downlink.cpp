#include "downlink.h"

#include <algorithm>
#include <cassert>

#include "common/dns.h"


namespace state {


std::shared_mutex Downlink::all_downlinks_mutex;
std::list<Downlink *> Downlink::all_downlinks;

void Downlink::broadcast(uint64_t notification_flag, const void *msg, size_t n) {
	std::shared_lock<std::shared_mutex> all_downlinks_rdlock(all_downlinks_mutex);
	for (auto downlink_ptr : all_downlinks) {
		if (downlink_ptr->notification_mask & notification_flag) {
			try {
				downlink_ptr->send_message(msg, n);
			}
			catch (...) {
				downlink_ptr->abort();
			}
		}
	}
}

void Downlink::broadcast(uint64_t notification_flag, const Sink::BufferPointer bufs[], size_t count) {
	std::shared_lock<std::shared_mutex> all_downlinks_rdlock(all_downlinks_mutex);
	for (auto downlink_ptr : all_downlinks) {
		if (downlink_ptr->notification_mask & notification_flag) {
			try {
				downlink_ptr->send_message(bufs, count);
			}
			catch (...) {
				downlink_ptr->abort();
			}
		}
	}
}

void Downlink::broadcast(uint64_t notification_mask, uint64_t notification_flags, const void *msg, size_t n) {
	std::shared_lock<std::shared_mutex> all_downlinks_rdlock(all_downlinks_mutex);
	for (auto downlink_ptr : all_downlinks) {
		if ((downlink_ptr->notification_mask & notification_mask) == notification_flags) {
			try {
				downlink_ptr->send_message(msg, n);
			}
			catch (...) {
				downlink_ptr->abort();
			}
		}
	}
}

Downlink::Downlink(Socket &&socket, const sockaddr_in6 &peer_addr, uint8_t min_protocol_version, uint8_t max_protocol_version)
	: Transceiver(std::move(socket)), peer_addr(peer_addr)
{
	{
		struct NegotiateProtocolVersion msg;
		msg.opcode = NegotiateProtocolVersion;
		msg.min_version = min_protocol_version;
		msg.max_version = max_protocol_version;
		this->send_message(&msg, sizeof msg);
	}
	std::lock_guard<std::shared_mutex> all_downlinks_wrlock(all_downlinks_mutex);
	all_downlinks_itr = all_downlinks.insert(all_downlinks.end(), this);
}

Downlink::~Downlink() {
	if (elog.debug_enabled()) {
		elog.debug() << "closing connection with " << peer_addr << std::endl;
	}
	std::lock_guard<std::shared_mutex> all_downlinks_wrlock(all_downlinks_mutex);
	all_downlinks.erase(all_downlinks_itr);
}

size_t Downlink::receive_message(const struct SetNotificationMask &msg, size_t n) {
	if (n < sizeof msg) {
		return 0;
	}
	auto new_flags = ~notification_mask & msg.mask;
	notification_mask = msg.mask;
	if (new_flags & (flag<BalanceChanged>() | flag<UserPublicKeyChanged>() | flag<UserTradeVolumeChanged>())) {
		walk_users([this, new_flags](id_t user_id, User &user) {
			std::lock_guard<std::mutex> user_lock(user.mutex);
			if ((new_flags & flag<UserPublicKeyChanged>()) && std::any_of(std::begin(user.public_key.x), std::end(user.public_key.x), [](uint8_t b) { return b != 0; })) {
				struct UserPublicKeyChanged notice;
				notice.opcode = UserPublicKeyChanged;
				notice.user_id = user_id;
				notice.public_key = user.public_key;
				this->send_message(&notice, sizeof notice);
			}
			if (new_flags & (flag<BalanceChanged>() | flag<UserTradeVolumeChanged>())) {
				user.walk_assets_locked([this, new_flags, user_id](asset_t asset_id, User::AssetInfo &asset_info) {
					if (new_flags & flag<BalanceChanged>()) {
						struct BalanceChanged notice;
						notice.opcode = BalanceChanged;
						notice.user_id = user_id;
						notice.balance.asset_id = asset_id;
						notice.balance.balance = asset_info.balance;
						this->send_message(&notice, sizeof notice);
					}
					if ((new_flags & flag<UserTradeVolumeChanged>()) && asset_info.trade_volume != 0) {
						struct UserTradeVolumeChanged notice;
						notice.opcode = UserTradeVolumeChanged;
						notice.user_id = user_id;
						notice.asset_id = asset_id;
						notice.volume = asset_info.trade_volume;
						this->send_message(&notice, sizeof notice);
					}
				});
			}
		});
	}
	if (new_flags & (flag<OrderOpened>() | flag<TickerChanged>() | flag<MatchPriceBoundsChanged>())) {
		walk_books([this, new_flags](const asset_pair_t &asset_pair, Book &book) {
			std::lock_guard<std::mutex> book_lock(book.mutex);
			if (new_flags & flag<MatchPriceBoundsChanged>()) {
				struct MatchPriceBoundsChanged notice;
				notice.opcode = MatchPriceBoundsChanged;
				notice.asset_pair = asset_pair;
				notice.lower_bound = book.match_price_lower_bound;
				notice.upper_bound = book.match_price_upper_bound;
				this->send_message(&notice, sizeof notice);
			}
			if (new_flags & flag<OrderOpened>()) {
				for (auto &pair : book.bids) {
					for (auto &order : pair.second) {
						this->send_order(order);
					}
				}
				for (auto &pair : book.asks) {
					for (auto &order : pair.second) {
						this->send_order(order);
					}
				}
			}
			if (new_flags & flag<TickerChanged>()) {
				auto notice = book.make_ticker_notice_locked(asset_pair, timestamp_t::clock::now());
				this->send_message(&notice, sizeof notice);
			}
		});
	}
	if (protocol_version >= 4) {
		Opcode opcode = Synchronized;
		this->send_message(&opcode, sizeof opcode);
	}
	return sizeof msg;
}

void Downlink::send_order(const OrderRecord &order) {
	struct OrderOpened notice;
	notice.opcode = OrderOpened;
	notice.user_id = order.user_id;
	notice.order.order_id = order.user_orders_by_id_itr->first;
	notice.order.order_info = order;
	this->send_message(&notice, sizeof notice);
}

} // namespace state
