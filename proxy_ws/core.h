#pragma once

#include <chrono>

#include <netinet/in.h>

#include "common/compiler.h"
#include "common/enumflags.h"

namespace core {

typedef uint64_t id_t;
typedef uint16_t asset_t;
typedef uint64_t asset_price_t;
typedef std::pair<asset_t, asset_t> asset_pair_t;
typedef std::chrono::system_clock::time_point timestamp_t;

static constexpr size_t _const hash(asset_pair_t asset_pair) {
#if BYTE_ORDER == BIG_ENDIAN
	return asset_pair.first << 16 | asset_pair.second;
#else
	return asset_pair.first | asset_pair.second << 16;
#endif
}

struct PublicKey {
	uint8_t x[28];
	uint8_t y[28];
};

static constexpr in_port_t CORE_PORT = 24931;
static constexpr uint8_t PROTOCOL_VERSION = 8;

static constexpr uint64_t PRICE_SCALE = 10000;
static constexpr uint64_t FEE_SCALE = 1000000;
static constexpr size_t MAX_ORDERS = 1000;
static constexpr size_t MAX_REQUESTS = 1000;

enum Opcode : uint8_t {
	GetUserPublicKey = 0x00,
	SetUserPublicKey,
	GetAssetAuthority,
	SetAssetAuthority,
	CreateBook,
	GetBalances,
	GetBalance,
	AdjustBalance,
	GetOrders,
	PlaceOrder,
	CancelOrder,
	LoadUserTradeVolume,
	StoreBooks,
	LoadBooks,
	SetFeeTable,
	SetUserFeeTables,
	ExecuteBaseMarketOrder,
	ExecuteCounterMarketOrder,
	GetTradeVolume,
	CancelAllOrders,
	LoadBookTradeHistory,
	DumpState,
	SetUserTonce,
	CancelOrderByTonce,
	CancelOrderNoReply,
	PlaceOrderEx, // v1
	TransferBalance, // v1
	SetMatchPriceBounds, // v1
	SetNextOrderID, // v3
	AdjustBalances, // v5
	ModifyOrder, // v6
	ModifyOrderByTonce, // v6

	Success = 0x80,
	NotFound,
	Exists,
	OutOfSequence,
	InsufficientFunds,
	TooMany,
	TooFast,
	InvalidArgument,
	NotAllowed,

	BalanceChanged = 0xC0,
	BalanceAdjusted,
	OrderOpened,
	OrdersMatched_v0,
	OrderClosed,
	TickerChanged,
	UserPublicKeyChanged,
	MatchPriceBoundsChanged, // v1
	UserTradeVolumeChanged, // v2
	BalancesAdjusted, // v5
	OrderModified, // v6
	OrdersMatched, // v8

	Synchronized = 0xFD, // v4
	SetNotificationMask = 0xFE,
	NegotiateProtocolVersion = 0xFF,
};

template <Opcode opcode>
struct flag : public std::integral_constant<uint64_t, UINT64_C(1) << (opcode & ~0xC0)> {
	static_assert(opcode >= BalanceChanged && opcode <= OrdersMatched, "must be notification opcode");
};

static constexpr uint64_t CRITICAL_CLIENT_FLAG = UINT64_C(1) << 63;

#pragma pack(push, 1)

struct Balance {
	asset_t asset_id;
	uint64_t balance;
};

enum class OrderOrigin : uint8_t {
	DEFAULT = 0,
	DAEMON  = 1,
	MOBILE  = 2,
	BRACKET = 3,
};

struct OrderFlags {
	uint8_t fee_external : 1; // v7
	uint8_t taker        : 1; // v8
	uint8_t reserved     : 4;
	uint8_t origin       : 2; // v8
	constexpr explicit _pure operator bool () const noexcept { return reinterpret_cast<const uint8_t &>(*this); }
};

struct OrderParams {
	uint64_t tonce;
	asset_pair_t asset_pair;
	int64_t quantity;
	uint64_t price;
	OrderFlags flags;
};

struct OrderInfo : OrderParams {
	timestamp_t time;

	OrderInfo() { }
	OrderInfo(const OrderParams &params) : OrderParams(params), time(timestamp_t::clock::now()) { }
};

struct Order {
	id_t order_id;
	struct OrderInfo order_info;
};


struct GetUserPublicKey {
	Opcode opcode;
	id_t user_id;
};

struct GetUserPublicKeyReply {
	Opcode opcode;
	PublicKey public_key;
};

struct SetUserPublicKey {
	Opcode opcode;
	id_t user_id;
	PublicKey public_key;
};

struct SetUserPublicKeyReply {
	Opcode opcode;
};

struct GetAssetAuthority {
	Opcode opcode;
	id_t user_id;
};

struct GetAssetAuthorityReply {
	Opcode opcode;
	uint16_t assets_count;
	asset_t asset_ids[];

	static void * operator new (size_t size, uint16_t assets_count) { return ::operator new (size + assets_count * sizeof *asset_ids); }
	constexpr size_t _pure size() const noexcept { return sizeof *this + assets_count * sizeof *asset_ids; }
};

struct SetAssetAuthority {
	Opcode opcode;
	id_t user_id;
	uint16_t assets_count;
	asset_t asset_ids[];

	static void * operator new (size_t size, uint16_t assets_count) { return ::operator new (size + assets_count * sizeof *asset_ids); }
	constexpr size_t _pure size() const noexcept { return sizeof *this + assets_count * sizeof *asset_ids; }
};

struct SetAssetAuthorityReply {
	Opcode opcode;
};

struct CreateBook {
	Opcode opcode;
	asset_pair_t asset_pair;
	uint64_t price_granularity;
};

struct CreateBookReply {
	Opcode opcode;
};

struct GetBalances {
	Opcode opcode;
	id_t user_id;
};

struct BalanceInfo {
    asset_t asset_id;
    uint64_t balance;
    uint64_t reserved_balance;
    uint64_t total_balance;
};

struct GetBalancesReply {
	Opcode opcode;
	uint16_t balances_count;
	struct BalanceInfo balances[];

	static void * operator new (size_t size, uint16_t balances_count) { return ::operator new (size + balances_count * sizeof *balances); }
	constexpr size_t _pure size() const noexcept { return sizeof *this + balances_count * sizeof *balances; }
};

struct GetBalance {
	Opcode opcode;
	id_t user_id;
	asset_t asset_id;
};

struct GetBalanceReply {
	Opcode opcode;
	uint64_t balance;
	uint64_t seq_num;
};

struct AdjustBalance {
	Opcode opcode;
	id_t user_id;
	asset_t asset_id;
	int64_t delta;
	uint64_t seq_num;
};

struct AdjustBalanceReply {
	Opcode opcode;
	uint64_t balance;
};

struct GetOrders {
	Opcode opcode;
	id_t user_id;
};

struct GetOrdersReply {
	Opcode opcode;
	uint16_t orders_count;
	struct Order orders[];

	static void * operator new (size_t size, uint16_t orders_count) { return ::operator new (size + orders_count * sizeof *orders); }
	constexpr size_t _pure size() const noexcept { return sizeof *this + orders_count * sizeof *orders; }
};

struct ExecuteMarketOrder {
	Opcode opcode;
	id_t user_id;
	uint64_t tonce;
	asset_pair_t asset_pair;
	int64_t quantity;
	OrderFlags flags;
};

struct ExecuteMarketOrderReply {
	Opcode opcode;
	int64_t remaining;
};

struct PlaceOrder {
	Opcode opcode;
	id_t user_id;
	struct OrderParams order_params;
};

struct PlaceOrderEx : PlaceOrder {
	enum Flags : uint8_t {
		ADMIN        = 1 << 0,
		FILL_OR_KILL = 1 << 1,
		POST_ONLY    = 1 << 2,
	} flags;
};
DEFINE_ENUM_FLAG_OPS(PlaceOrderEx::Flags)

struct PlaceOrderReply {
	Opcode opcode;
	id_t order_id;
	timestamp_t time;
};

struct ModifyOrder {
	Opcode opcode;
	id_t user_id;
	union {
		id_t order_id;
		uint64_t tonce;
	};
	int64_t quantity_delta;
	uint64_t price;
	PlaceOrderEx::Flags flags;
};

struct ModifyOrderReply {
	Opcode opcode;
	struct Order order;
};

struct CancelOrder {
	Opcode opcode;
	id_t user_id;
	union {
		id_t order_id;
		uint64_t tonce;
	};
};

struct CancelOrderReply {
	Opcode opcode;
	struct Order order;
};

struct LoadUserTradeVolume {
	Opcode opcode;
	id_t user_id;
	asset_t asset_id;
	uint16_t trades_count;
	struct VolumeInfo {
		uint64_t quantity;
		timestamp_t time;
	} trades[];

	static void * operator new (size_t size, uint16_t trades_count) { return ::operator new (size + trades_count * sizeof *trades); }
	constexpr size_t _pure size() const noexcept { return sizeof *this + trades_count * sizeof *trades; }
};

struct LoadUserTradeVolumeReply {
	Opcode opcode;
	uint64_t volume;
};

struct StoreBooks {
	Opcode opcode;
};

struct StoreBooksReply {
	Opcode opcode;
};

struct LoadBooks {
	Opcode opcode;
};

struct LoadBooksReply {
	Opcode opcode;
};

struct SetFeeTable {
	Opcode opcode;
	uint16_t table_id;
	asset_t asset_id;
	uint16_t entries_count;
	struct FeeInfo {
		uint64_t volume;
		uint64_t fee;
	} entries[];

	static void * operator new (size_t size, uint16_t entries_count) { return ::operator new (size + entries_count * sizeof *entries); }
	constexpr size_t _pure size() const noexcept { return sizeof *this + entries_count * sizeof *entries; }
};

struct SetFeeTableReply {
	Opcode opcode;
};

struct SetUserFeeTables {
	Opcode opcode;
	id_t user_id;
	uint16_t maker_table_id;
	uint16_t taker_table_id;
};

struct SetUserFeeTablesReply {
	Opcode opcode;
};

struct GetTradeVolume {
	Opcode opcode;
	id_t user_id;
	asset_t asset_id;
};

struct GetTradeVolumeReply {
	Opcode opcode;
	uint64_t volume;
};

struct CancelAllOrders {
	Opcode opcode;
	id_t user_id;
};

struct CancelAllOrdersReply {
	Opcode opcode;
	uint16_t orders_count;
	struct Order orders[];

	static void * operator new (size_t size, uint16_t orders_count) { return ::operator new (size + orders_count * sizeof *orders); }
	constexpr size_t _pure size() const noexcept { return sizeof *this + orders_count * sizeof *orders; }
};

struct LoadBookTradeHistory {
	Opcode opcode;
	asset_pair_t asset_pair;
	uint16_t trades_count;
	struct TradeInfo {
		uint64_t quantity;
		uint64_t price;
		timestamp_t time;
	} trades[];

	static void * operator new (size_t size, uint16_t trades_count) { return ::operator new (size + trades_count * sizeof *trades); }
	constexpr size_t _pure size() const noexcept { return sizeof *this + trades_count * sizeof *trades; }
};

struct LoadBookTradeHistoryReply {
	Opcode opcode;
};

struct DumpState {
	Opcode opcode;
};

struct DumpStateReply {
	Opcode opcode;
};

struct SetUserTonce {
	Opcode opcode;
	id_t user_id;
	uint64_t tonce;
};

struct SetUserTonceReply {
	Opcode opcode;
};

struct TransferBalance {
	Opcode opcode;
	id_t from_user_id;
	id_t to_user_id;
	asset_t asset_id;
	uint64_t amount;
	uint64_t from_seq_num;
	uint64_t to_seq_num;
};

struct TransferBalanceReply {
	Opcode opcode;
	uint64_t from_balance;
	uint64_t to_balance;
};

struct SetMatchPriceBounds {
	Opcode opcode;
	asset_pair_t asset_pair;
	uint64_t lower_bound;
	uint64_t upper_bound;
};

struct SetMatchPriceBoundsReply {
	Opcode opcode;
};

struct SetNextOrderID {
	Opcode opcode;
	id_t next_order_id;
};

struct SetNextOrderIDReply {
	Opcode opcode;
};

enum class DetailsType : uint8_t {
	NONE, // 0 bytes
	BLOB,
	INTEGER, // 1-8 bytes, little endian
	REAL, // 4 or 8 bytes, native byte order
	STRING,
	JSON,
};

struct AdjustBalances {
	Opcode opcode;
	uint16_t adjustments_count;
	DetailsType details_type;
	uint16_t details_size;
	struct Adjustment {
		id_t user_id;
		asset_t asset_id;
		int64_t delta;
		uint64_t seq_num;
	} adjustments[];

	static void * operator new (size_t size, uint16_t adjustments_count, uint16_t details_size) { return ::operator new (size + adjustments_count * sizeof *adjustments + details_size); }
	constexpr size_t _pure size() const noexcept { return sizeof *this + adjustments_count * sizeof *adjustments + details_size; }

	constexpr void * _pure details() noexcept { return adjustments + adjustments_count; }
	constexpr const void * _pure details() const noexcept { return adjustments + adjustments_count; }
};

struct AdjustBalancesReply {
	Opcode opcode;
};


struct BalanceChanged {
	Opcode opcode;
	id_t user_id;
	struct Balance balance;
};

struct BalanceAdjusted : BalanceChanged {
	uint64_t seq_num;
};

struct OrderOpened {
	Opcode opcode;
	id_t user_id;
	struct Order order;
};

struct OrderModified {
	Opcode opcode;
	id_t user_id;
	struct Order order;
};

struct OrdersMatched_v0 {
	Opcode opcode;
	id_t bid_user_id;
	id_t bid_order_id;
	uint64_t bid_tonce;
	id_t ask_user_id;
	id_t ask_order_id;
	uint64_t ask_tonce;
	asset_pair_t asset_pair;
	uint64_t quantity;
	uint64_t price;
	uint64_t total;
	uint64_t bid_base_fee;
	uint64_t bid_counter_fee;
	uint64_t ask_base_fee;
	uint64_t ask_counter_fee;
	uint64_t bid_remaining_quantity;
	uint64_t ask_remaining_quantity;
	timestamp_t time;
};

struct OrdersMatched : OrdersMatched_v0 {
	OrderFlags bid_flags;
	OrderFlags ask_flags;
};

struct OrderClosed {
	Opcode opcode;
	id_t user_id;
	struct Order order;
    timestamp_t time_closed;

    OrderClosed() : time_closed(timestamp_t::clock::now()){}
};

struct TickerChanged {
	Opcode opcode;
	asset_pair_t asset_pair;
	uint64_t last;
	uint64_t bid;
	uint64_t ask;
	uint64_t low_24h;
	uint64_t high_24h;
	uint64_t volume_24h;
	timestamp_t time;
};

struct UserPublicKeyChanged {
	Opcode opcode;
	id_t user_id;
	PublicKey public_key;
};

struct MatchPriceBoundsChanged {
	Opcode opcode;
	asset_pair_t asset_pair;
	uint64_t lower_bound;
	uint64_t upper_bound;
};

struct UserTradeVolumeChanged {
	Opcode opcode;
	id_t user_id;
	asset_t asset_id;
	uint64_t volume;
};

struct BalancesAdjusted {
	Opcode opcode;
	uint16_t adjustments_count;
	DetailsType details_type;
	uint16_t details_size;
	struct Adjustment {
		id_t user_id;
		asset_t asset_id;
		uint64_t balance;
		uint64_t seq_num;
	} adjustments[];

	static void * operator new (size_t size, uint16_t adjustments_count, uint16_t details_size) { return ::operator new (size + adjustments_count * sizeof *adjustments + details_size); }
	constexpr size_t _pure size() const noexcept { return sizeof *this + adjustments_count * sizeof *adjustments + details_size; }

	constexpr void * _pure details() noexcept { return adjustments + adjustments_count; }
	constexpr const void * _pure details() const noexcept { return adjustments + adjustments_count; }
};


struct SetNotificationMask {
	Opcode opcode;
	uint64_t mask;
};

struct NegotiateProtocolVersion {
	Opcode opcode;
	uint8_t min_version;
	uint8_t max_version;
};

#pragma pack(pop)

template <typename M, typename = void>
struct message_size {
	constexpr size_t _const operator () (const void *, size_t) const noexcept { return sizeof(M); }
};

template <typename M>
struct message_size<M, std::void_t<decltype(std::declval<M>().size())>> {
	constexpr size_t _pure operator () (const void *buf, size_t n) const noexcept(noexcept(static_cast<size_t>(static_cast<const M *>(buf)->size()))) {
		return n < sizeof(M) ? sizeof(M) : static_cast<const M *>(buf)->size();
	}
};


std::ostream & operator << (std::ostream &os, Opcode opcode);

std::ostream & operator << (std::ostream &os, const asset_pair_t &asset_pair);
std::ostream & operator << (std::ostream &os, const struct Balance &balance);
std::ostream & operator << (std::ostream &os, const struct BalanceInfo &balance_info);
std::ostream & operator << (std::ostream &os, const struct OrderParams &order_params);
std::ostream & operator << (std::ostream &os, const struct OrderInfo &order_info);
std::ostream & operator << (std::ostream &os, const struct Order &order);

std::ostream & operator << (std::ostream &os, const struct GetUserPublicKey &msg);
std::ostream & operator << (std::ostream &os, const struct GetUserPublicKeyReply &msg);
std::ostream & operator << (std::ostream &os, const struct SetUserPublicKey &msg);
std::ostream & operator << (std::ostream &os, const struct SetUserPublicKeyReply &msg);
std::ostream & operator << (std::ostream &os, const struct GetAssetAuthority &msg);
std::ostream & operator << (std::ostream &os, const struct GetAssetAuthorityReply &msg);
std::ostream & operator << (std::ostream &os, const struct SetAssetAuthority &msg);
std::ostream & operator << (std::ostream &os, const struct SetAssetAuthorityReply &msg);
std::ostream & operator << (std::ostream &os, const struct CreateBook &msg);
std::ostream & operator << (std::ostream &os, const struct CreateBookReply &msg);
std::ostream & operator << (std::ostream &os, const struct GetBalances &msg);
std::ostream & operator << (std::ostream &os, const struct GetBalancesReply &msg);
std::ostream & operator << (std::ostream &os, const struct GetBalance &msg);
std::ostream & operator << (std::ostream &os, const struct GetBalanceReply &msg);
std::ostream & operator << (std::ostream &os, const struct AdjustBalance &msg);
std::ostream & operator << (std::ostream &os, const struct AdjustBalanceReply &msg);
std::ostream & operator << (std::ostream &os, const struct GetOrders &msg);
std::ostream & operator << (std::ostream &os, const struct GetOrdersReply &msg);
std::ostream & operator << (std::ostream &os, const struct PlaceOrder &msg);
std::ostream & operator << (std::ostream &os, const struct PlaceOrderEx &msg);
std::ostream & operator << (std::ostream &os, const struct PlaceOrderReply &msg);
std::ostream & operator << (std::ostream &os, const struct ModifyOrder &msg);
std::ostream & operator << (std::ostream &os, const struct ModifyOrderReply &msg);
std::ostream & operator << (std::ostream &os, const struct CancelOrder &msg);
std::ostream & operator << (std::ostream &os, const struct CancelOrderReply &msg);
std::ostream & operator << (std::ostream &os, const struct LoadUserTradeVolume &msg);
std::ostream & operator << (std::ostream &os, const struct LoadUserTradeVolumeReply &msg);
std::ostream & operator << (std::ostream &os, const struct StoreBooks &msg);
std::ostream & operator << (std::ostream &os, const struct StoreBooksReply &msg);
std::ostream & operator << (std::ostream &os, const struct LoadBooks &msg);
std::ostream & operator << (std::ostream &os, const struct LoadBooksReply &msg);
std::ostream & operator << (std::ostream &os, const struct SetFeeTable &msg);
std::ostream & operator << (std::ostream &os, const struct SetFeeTableReply &msg);
std::ostream & operator << (std::ostream &os, const struct SetUserFeeTables &msg);
std::ostream & operator << (std::ostream &os, const struct SetUserFeeTablesReply &msg);
std::ostream & operator << (std::ostream &os, const struct ExecuteMarketOrder &msg);
std::ostream & operator << (std::ostream &os, const struct ExecuteMarketOrderReply &msg);
std::ostream & operator << (std::ostream &os, const struct GetTradeVolume &msg);
std::ostream & operator << (std::ostream &os, const struct GetTradeVolumeReply &msg);
std::ostream & operator << (std::ostream &os, const struct CancelAllOrders &msg);
std::ostream & operator << (std::ostream &os, const struct CancelAllOrdersReply &msg);
std::ostream & operator << (std::ostream &os, const struct LoadBookTradeHistory &msg);
std::ostream & operator << (std::ostream &os, const struct LoadBookTradeHistoryReply &msg);
std::ostream & operator << (std::ostream &os, const struct DumpState &msg);
std::ostream & operator << (std::ostream &os, const struct DumpStateReply &msg);
std::ostream & operator << (std::ostream &os, const struct SetUserTonce &msg);
std::ostream & operator << (std::ostream &os, const struct SetUserTonceReply &msg);
std::ostream & operator << (std::ostream &os, const struct TransferBalance &msg);
std::ostream & operator << (std::ostream &os, const struct TransferBalanceReply &msg);
std::ostream & operator << (std::ostream &os, const struct SetMatchPriceBounds &msg);
std::ostream & operator << (std::ostream &os, const struct SetMatchPriceBoundsReply &msg);
std::ostream & operator << (std::ostream &os, const struct SetNextOrderID &msg);
std::ostream & operator << (std::ostream &os, const struct SetNextOrderIDReply &msg);
std::ostream & operator << (std::ostream &os, const struct AdjustBalances &msg);
std::ostream & operator << (std::ostream &os, const struct AdjustBalancesReply &msg);

std::ostream & operator << (std::ostream &os, const struct BalanceChanged &msg);
std::ostream & operator << (std::ostream &os, const struct BalanceAdjusted &msg);
std::ostream & operator << (std::ostream &os, const struct OrderOpened &msg);
std::ostream & operator << (std::ostream &os, const struct OrderModified &msg);
std::ostream & operator << (std::ostream &os, const struct OrdersMatched_v0 &msg);
std::ostream & operator << (std::ostream &os, const struct OrdersMatched &msg);
std::ostream & operator << (std::ostream &os, const struct OrderClosed &msg);
std::ostream & operator << (std::ostream &os, const struct TickerChanged &msg);
std::ostream & operator << (std::ostream &os, const struct UserPublicKeyChanged &msg);
std::ostream & operator << (std::ostream &os, const struct MatchPriceBoundsChanged &msg);
std::ostream & operator << (std::ostream &os, const struct UserTradeVolumeChanged &msg);
std::ostream & print(std::ostream &os, const struct BalancesAdjusted &msg, std::string_view details);
std::ostream & operator << (std::ostream &os, const struct BalancesAdjusted &msg);


} // namespace core
