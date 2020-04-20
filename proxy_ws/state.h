#include <functional>
#include <list>
#include <map>
#include <mutex>

#include "core.h"


namespace state {


using namespace core;
using core::id_t;


struct OrderRecord : OrderInfo {
	id_t user_id;
	std::map<id_t, std::list<OrderRecord>::iterator>::const_iterator user_orders_by_id_itr;
	std::multimap<uint64_t, std::list<OrderRecord>::iterator>::const_iterator user_orders_by_tonce_itr;
	union {
		std::map<uint64_t, std::list<OrderRecord>, std::greater<uint64_t>>::iterator book_bids_itr;
		std::map<uint64_t, std::list<OrderRecord>>::iterator book_asks_itr;
	};

	using OrderInfo::OrderInfo;
	OrderRecord(const struct OrderOpened &msg) noexcept : OrderInfo(msg.order.order_info), user_id(msg.user_id) { }
};


struct User {
	struct AssetInfo {
		uint64_t balance = 0, seq_num = 0;
		uint64_t trade_volume = 0;

		void balance_changed_locked(const struct BalanceChanged &msg) {
			balance = msg.balance.balance;
		}

		void balance_changed_locked(const struct BalanceAdjusted &msg) {
			balance = msg.balance.balance;
			seq_num = msg.seq_num;
		}

		void trade_volume_changed_locked(const struct UserTradeVolumeChanged &msg) {
			trade_volume = msg.volume;
		}
	};

	std::mutex mutex;
	PublicKey public_key;
	std::map<id_t, std::list<OrderRecord>::iterator> orders_by_id;
	std::multimap<uint64_t, std::list<OrderRecord>::iterator> orders_by_tonce;

	User() : public_key() { }
	explicit User(const PublicKey &public_key) : public_key(public_key) { }
	virtual ~User() = default;

	virtual void walk_assets_locked(const std::function<void (asset_t asset_id, AssetInfo &)> &visitor) = 0;
};

std::array<std::unique_lock<std::mutex>, 2> lock_users(id_t user1_id, User &user1, id_t user2_id, User &user2);

void walk_users(const std::function<void (id_t user_id, User &)> &visitor);


struct Book {
	std::mutex mutex;
	std::map<uint64_t, std::list<OrderRecord>, std::greater<uint64_t>> bids;
	std::map<uint64_t, std::list<OrderRecord>> asks;
	uint64_t match_price_lower_bound = 0, match_price_upper_bound = UINT64_MAX;
	uint64_t last = 0, low_24h = 0, high_24h = 0, volume_24h = 0;

	void order_opened_locked(User &user, const struct OrderOpened &msg);
	void order_modified_locked(User &user, const struct OrderModified &msg);
	void order_modified_locked(std::list<OrderRecord>::iterator order_itr, const struct OrderModified &msg);
	void orders_matched_locked(User &bid_user, User &ask_user, const struct OrdersMatched &msg);
	void order_closed_locked(User &user, const struct OrderClosed &msg);
	void order_closed_locked(User &user, std::list<OrderRecord>::const_iterator order_itr);
	void ticker_changed_locked(const struct TickerChanged &msg);

	struct TickerChanged make_ticker_notice_locked(const asset_pair_t &asset_pair, timestamp_t) const;
};

void walk_books(const std::function<void (const asset_pair_t &asset_pair, Book &)> &visitor);


} // namespace state
