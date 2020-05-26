#include "core.h"

#include <iomanip>
#include <string_view>

namespace {

template <typename T>
struct _pretty {
};

template <typename T>
_pretty<T> _pure pretty(T value) noexcept {
	return _pretty<T>{ value };
}

template <>
struct _pretty<core::PlaceOrderEx::Flags> {
	friend _pretty<core::PlaceOrderEx::Flags> pretty(core::PlaceOrderEx::Flags);
	core::PlaceOrderEx::Flags flags;
	friend std::ostream & operator << (std::ostream &os, _pretty<core::PlaceOrderEx::Flags> pp) {
		if (pp.flags) {
			if (pp.flags & core::PlaceOrderEx::ADMIN) {
				os << "ADMIN";
				if (pp.flags >> 1) {
					os << " | ";
				}
			}
			if (pp.flags & core::PlaceOrderEx::FILL_OR_KILL) {
				os << "FILL_OR_KILL";
			}
		}
		else {
			os << '0';
		}
		return os;
	}
};

} // anonymous namespace

namespace std { // polyfill for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=86008 (fixed in GCC 8.2)
	template <typename CharT, typename Traits, typename = void>
	struct has_quoted_string_view : false_type { };
	template <typename CharT, typename Traits>
	struct has_quoted_string_view<CharT, Traits, void_t<decltype(std::quoted(basic_string_view<CharT, Traits>{ }))>> : true_type { };
	template <typename CharT, typename Traits, typename Enable = enable_if_t<!has_quoted_string_view<CharT, Traits>::value>>
	inline auto quoted(const basic_string_view<CharT, Traits> &sv, CharT delim = CharT('"'), CharT escape = CharT('\\')) {
		return __detail::_Quoted_string<const basic_string_view<CharT, Traits> &, CharT>(sv, delim, escape);
	}
}

namespace core {

static const char * _const opcode_name(Opcode opcode) noexcept {
	switch (opcode) {
		case GetUserPublicKey:
			return "GetUserPublicKey";
		case SetUserPublicKey:
			return "SetUserPublicKey";
		case GetAssetAuthority:
			return "GetAssetAuthority";
		case SetAssetAuthority:
			return "SetAssetAuthority";
		case CreateBook:
			return "CreateBook";
		case GetBalances:
			return "GetBalances";
		case GetBalance:
			return "GetBalance";
		case AdjustBalance:
			return "AdjustBalance";
		case GetOrders:
			return "GetOrders";
		case PlaceOrder:
			return "PlaceOrder";
		case CancelOrder:
			return "CancelOrder";
		case LoadUserTradeVolume:
			return "LoadUserTradeVolume";
		case StoreBooks:
			return "StoreBooks";
		case LoadBooks:
			return "LoadBooks";
		case SetFeeTable:
			return "SetFeeTable";
		case SetUserFeeTables:
			return "SetUserFeeTables";
		case ExecuteBaseMarketOrder:
			return "ExecuteBaseMarketOrder";
		case ExecuteCounterMarketOrder:
			return "ExecuteCounterMarketOrder";
		case GetTradeVolume:
			return "GetTradeVolume";
		case CancelAllOrders:
			return "CancelAllOrders";
		case LoadBookTradeHistory:
			return "LoadBookTradeHistory";
		case DumpState:
			return "DumpState";
		case SetUserTonce:
			return "SetUserTonce";
		case CancelOrderByTonce:
			return "CancelOrderByTonce";
		case CancelOrderNoReply:
			return "CancelOrderNoReply";
		case PlaceOrderEx:
			return "PlaceOrderEx";
		case TransferBalance:
			return "TransferBalance";
		case SetMatchPriceBounds:
			return "SetMatchPriceBounds";
		case SetNextOrderID:
			return "SetNextOrderID";
		case AdjustBalances:
			return "AdjustBalances";
		case ModifyOrder:
			return "ModifyOrder";
		case ModifyOrderByTonce:
			return "ModifyOrderByTonce";
		case Success:
			return "Success";
		case NotFound:
			return "NotFound";
		case Exists:
			return "Exists";
		case OutOfSequence:
			return "OutOfSequence";
		case InsufficientFunds:
			return "InsufficientFunds";
		case TooMany:
			return "TooMany";
		case TooFast:
			return "TooFast";
		case InvalidArgument:
			return "InvalidArgument";
		case NotAllowed:
			return "NotAllowed";
		case BalanceChanged:
			return "BalanceChanged";
		case BalanceAdjusted:
			return "BalanceAdjusted";
		case OrderOpened:
			return "OrderOpened";
		case OrdersMatched_v0:
		case OrdersMatched:
			return "OrdersMatched";
		case OrderClosed:
			return "OrderClosed";
		case TickerChanged:
			return "TickerChanged";
		case UserPublicKeyChanged:
			return "UserPublicKeyChanged";
		case MatchPriceBoundsChanged:
			return "MatchPriceBoundsChanged";
		case UserTradeVolumeChanged:
			return "UserTradeVolumeChanged";
		case BalancesAdjusted:
			return "BalancesAdjusted";
		case OrderModified:
			return "OrderModified";
		case Synchronized:
			return "Synchronized";
		case SetNotificationMask:
			return "SetNotificationMask";
		case NegotiateProtocolVersion:
			return "NegotiateProtocolVersion";
	}
	return nullptr;
}

std::ostream & operator << (std::ostream &os, Opcode opcode) {
	const char *name = opcode_name(opcode);
	return name ? os << name : os << opcode;
}


std::ostream & operator << (std::ostream &os, const asset_pair_t &asset_pair) {
	return os << "{ " << std::showbase << std::hex << asset_pair.first << ", " << asset_pair.second << std::dec << " }";
}

std::ostream & operator << (std::ostream &os, const struct Balance &balance) {
	return os << "{ .asset_id = " << std::showbase << std::hex << balance.asset_id << std::dec << ", .balance = " << balance.balance << " }";
}

std::ostream & operator << (std::ostream &os, const struct BalanceInfo &balance_info) {
        return os << "{ .asset_id = " << std::showbase << std::hex << balance_info.asset_id << std::dec << ", .balance = " << balance_info.balance << ", .reserved balance = " << balance_info.reserved_balance << ", .total balance = " << balance_info.total_balance << " }";
}

static std::ostream & operator << (std::ostream &os, OrderOrigin origin) {
	switch (origin) {
		case OrderOrigin::DEFAULT:
			return os << "DEFAULT";
		case OrderOrigin::DAEMON:
			return os << "DAEMON";
		case OrderOrigin::MOBILE:
			return os << "MOBILE";
		case OrderOrigin::BRACKET:
			return os << "BRACKET";
	}
	return os << std::showbase << std::hex << +static_cast<std::underlying_type_t<decltype(origin)>>(origin) << std::dec;
}

static std::ostream & operator << (std::ostream &os, const struct OrderFlags &order_flags) {
	os << '{';
	if (order_flags.fee_external) {
		os << " .fee_external = " << +order_flags.fee_external;
	}
	if (order_flags.origin) {
		os << (order_flags.fee_external ? ", .origin = " : " .origin = ") << static_cast<OrderOrigin>(order_flags.origin);
	}
	return os << " }";
}

static std::ostream & print_base(std::ostream &os, const struct OrderParams &order_params) {
	os << "{ .tonce = " << order_params.tonce << ", .asset_pair = " << order_params.asset_pair << ", .quantity = " << order_params.quantity << ", .price = " << order_params.price;
	if (order_params.flags) {
		os << ", .flags = " << order_params.flags;
	}
	return os;
}

std::ostream & operator << (std::ostream &os, const struct OrderParams &order_params) {
	return print_base(os, order_params) << " }";
}

std::ostream & operator << (std::ostream &os, const struct OrderInfo &order_info) {
	return print_base(os, order_info) << ", .time = " << order_info.time.time_since_epoch().count() << " }";
}

std::ostream & operator << (std::ostream &os, const struct Order &order) {
	return os << "{ .order_id = " << order.order_id << ", .order_info = " << order.order_info << " }";
}


std::ostream & operator << (std::ostream &os, const struct GetUserPublicKey &cmd) {
	return os << GetUserPublicKey << "{ .user_id = " << cmd.user_id << " }";
}

std::ostream & operator << (std::ostream &os, const struct GetUserPublicKeyReply &reply) {
	os << GetUserPublicKey << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .public_key = [ ... ]";
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct SetUserPublicKey &cmd) {
	return os << SetUserPublicKey << "{ .user_id = " << cmd.user_id << ", .public_key = [ ... ] }";
}

std::ostream & operator << (std::ostream &os, const struct SetUserPublicKeyReply &reply) {
	return os << SetUserPublicKey << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct GetAssetAuthority &cmd) {
	return os << GetAssetAuthority << "{ .user_id = " << cmd.user_id << " }";
}

std::ostream & operator << (std::ostream &os, const struct GetAssetAuthorityReply &reply) {
	os << GetAssetAuthority << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .asset_ids = [" << std::showbase << std::hex;
		for (size_t i = 0; i < reply.assets_count; ++i) {
			os << (i == 0 ? " " : ", ") << reply.asset_ids[i];
		}
		os << std::dec << " ]";
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct SetAssetAuthority &cmd) {
	os << SetAssetAuthority << "{ .user_id = " << cmd.user_id << ", .asset_ids = [" << std::showbase << std::hex;
	for (size_t i = 0; i < cmd.assets_count; ++i) {
		os << (i == 0 ? " " : ", ") << cmd.asset_ids[i];
	}
	return os << std::dec << " ] }";
}

std::ostream & operator << (std::ostream &os, const struct SetAssetAuthorityReply &reply) {
	return os << SetAssetAuthority << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct CreateBook &cmd) {
	return os << CreateBook << "{ .asset_pair = " << cmd.asset_pair << ", .price_granularity = " << cmd.price_granularity << " }";
}

std::ostream & operator << (std::ostream &os, const struct CreateBookReply &reply) {
	return os << CreateBook << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct GetBalances &cmd) {
	return os << GetBalances << "{ .user_id = " << cmd.user_id << " }";
}

std::ostream & operator << (std::ostream &os, const struct GetBalancesReply &reply) {
	os << GetBalances << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .balances = [";
		for (size_t i = 0; i < reply.balances_count; ++i) {
			os << (i == 0 ? " " : ", ") << reply.balances[i];
		}
		os << " ]";
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct GetBalance &cmd) {
	return os << GetBalance << "{ .user_id = " << cmd.user_id << ", .asset_id = " << std::showbase << std::hex << cmd.asset_id << std::dec << " }";
}

std::ostream & operator << (std::ostream &os, const struct GetBalanceReply &reply) {
	os << GetBalance << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .balance = " << reply.balance << ", .seq_num = " << reply.seq_num;
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct AdjustBalance &cmd) {
	return os << AdjustBalance << "{ .user_id = " << cmd.user_id << ", .asset_id = " << std::showbase << std::hex << cmd.asset_id << std::dec << ", .delta = " << cmd.delta << ", .seq_num = " << cmd.seq_num << " }";
}

std::ostream & operator << (std::ostream &os, const struct AdjustBalanceReply &reply) {
	os << AdjustBalance << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .balance = " << reply.balance;
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct GetOrders &cmd) {
	return os << GetOrders << "{ .user_id = " << cmd.user_id << " }";
}

std::ostream & operator << (std::ostream &os, const struct GetOrdersReply &reply) {
	os << GetOrders << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .orders = [";
		for (size_t i = 0; i < reply.orders_count; ++i) {
			os << (i == 0 ? " " : ", ") << reply.orders[i];
		}
		os << " ]";
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct PlaceOrder &cmd) {
	return os << PlaceOrder << "{ .user_id = " << cmd.user_id << ", .order_params = " << cmd.order_params << " }";
}

std::ostream & operator << (std::ostream &os, const struct PlaceOrderEx &cmd) {
	return os << PlaceOrderEx << "{ .user_id = " << cmd.user_id << ", .order_params = " << cmd.order_params << ", .flags = " << pretty(cmd.flags) << " }";
}

std::ostream & operator << (std::ostream &os, const struct PlaceOrderReply &reply) {
	os << PlaceOrder << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .order_id = " << reply.order_id << ", .time = " << reply.time.time_since_epoch().count();
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct ModifyOrder &cmd) {
	os << cmd.opcode << "{ .user_id = " << cmd.user_id;
	if (cmd.opcode != ModifyOrderByTonce) {
		os << ", .order_id = " << cmd.order_id;
	}
	else {
		os << ", .tonce = " << cmd.tonce;
	}
	return os << ", .quantity_delta = " << cmd.quantity_delta << ", .price = " << cmd.price << ", .flags = " << pretty(cmd.flags) << " }";
}

std::ostream & operator << (std::ostream &os, const struct ModifyOrderReply &reply) {
	os << ModifyOrder << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .order = " << reply.order;
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct CancelOrder &cmd) {
	os << cmd.opcode << "{ .user_id = " << cmd.user_id;
	if (cmd.opcode != CancelOrderByTonce) {
		os << ", .order_id = " << cmd.order_id;
	}
	else {
		os << ", .tonce = " << cmd.tonce;
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct CancelOrderReply &reply) {
	os << CancelOrder << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .order = " << reply.order;
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct LoadUserTradeVolume &cmd) {
	os << LoadUserTradeVolume << "{ .user_id = " << cmd.user_id << ", .asset_id = " << std::showbase << std::hex << cmd.asset_id << std::dec << ", .trades = [";
	for (size_t i = 0; i < cmd.trades_count; ++i) {
		auto &volume_info = cmd.trades[i];
		os << (i == 0 ? " " : ", ") << "{ .quantity = " << volume_info.quantity << ", .time = " << volume_info.time.time_since_epoch().count() << " }";
	}
	return os << " ] }";
}

std::ostream & operator << (std::ostream &os, const struct LoadUserTradeVolumeReply &reply) {
	os << LoadUserTradeVolume << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .volume = " << reply.volume;
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct StoreBooks &) {
	return os << StoreBooks << "{ }";
}

std::ostream & operator << (std::ostream &os, const struct StoreBooksReply &reply) {
	return os << StoreBooks << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct LoadBooks &) {
	return os << LoadBooks << "{ }";
}

std::ostream & operator << (std::ostream &os, const struct LoadBooksReply &reply) {
	return os << LoadBooks << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct SetFeeTable &cmd) {
	os << SetFeeTable << "{ .table_id = " << cmd.table_id << ", .asset_id = " << std::showbase << std::hex << cmd.asset_id << std::dec << ", .entries = [";
	for (size_t i = 0; i < cmd.entries_count; ++i) {
		auto &fee_info = cmd.entries[i];
		os << (i == 0 ? " " : ", ") << "{ .volume = " << fee_info.volume << ", .fee = " << fee_info.fee << " }";
	}
	return os << " ] }";
}

std::ostream & operator << (std::ostream &os, const struct SetFeeTableReply &reply) {
	return os << SetFeeTable << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct SetUserFeeTables &cmd) {
	return os << SetUserFeeTables << "{ .user_id = " << cmd.user_id << ", .maker_table_id = " << cmd.maker_table_id << ", .taker_table_id = " << cmd.taker_table_id << " }";
}

std::ostream & operator << (std::ostream &os, const struct SetUserFeeTablesReply &reply) {
	return os << SetUserFeeTables << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct ExecuteMarketOrder &cmd) {
	return os << cmd.opcode << "{ .user_id = " << cmd.user_id << ", .tonce = " << cmd.tonce << ", .asset_pair = " << cmd.asset_pair << ", .quantity = " << cmd.quantity << " }";
}

std::ostream & operator << (std::ostream &os, const struct ExecuteMarketOrderReply &reply) {
	os << "ExecuteMarketOrder" << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .remaining = " << reply.remaining;
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct GetTradeVolume &cmd) {
	return os << GetTradeVolume << "{ .user_id = " << cmd.user_id << ", .asset_id = " << std::showbase << std::hex << cmd.asset_id << std::dec << " }";
}

std::ostream & operator << (std::ostream &os, const struct GetTradeVolumeReply &reply) {
	os << GetTradeVolume << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .volume = " << reply.volume;
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct CancelAllOrders &cmd) {
	return os << CancelAllOrders << "{ .user_id = " << cmd.user_id << " }";
}

std::ostream & operator << (std::ostream &os, const struct CancelAllOrdersReply &reply) {
	os << CancelAllOrders << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .orders = [";
		for (size_t i = 0; i < reply.orders_count; ++i) {
			os << (i == 0 ? " " : ", ") << reply.orders[i];
		}
		os << " ]";
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct LoadBookTradeHistory &cmd) {
	os << LoadBookTradeHistory << "{ .asset_pair = " << cmd.asset_pair << ", .trades = [";
	for (size_t i = 0; i < cmd.trades_count; ++i) {
		auto &trade_info = cmd.trades[i];
		os << (i == 0 ? " " : ", ") << "{ .quantity = " << trade_info.quantity << ", .price = " << trade_info.price << ", .time = " << trade_info.time.time_since_epoch().count() << " }";
	}
	return os << " ] }";
}

std::ostream & operator << (std::ostream &os, const struct LoadBookTradeHistoryReply &reply) {
	return os << LoadBookTradeHistory << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct DumpState &) {
	return os << DumpState << "{ }";
}

std::ostream & operator << (std::ostream &os, const struct DumpStateReply &reply) {
	return os << DumpState << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct SetUserTonce &cmd) {
	return os << SetUserTonce << "{ .user_id = " << cmd.user_id << ", .tonce = " << cmd.tonce << " }";
}

std::ostream & operator << (std::ostream &os, const struct SetUserTonceReply &reply) {
	return os << SetUserTonce << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct TransferBalance &cmd) {
	return os << TransferBalance << "{ .from_user_id = " << cmd.from_user_id << ", .to_user_id = " << cmd.to_user_id << ", .asset_id = " << std::showbase << std::hex << cmd.asset_id << std::dec << ", .amount = " << cmd.amount << ", .from_seq_num = " << cmd.from_seq_num << ", .to_seq_num = " << cmd.to_seq_num << " }";
}

std::ostream & operator << (std::ostream &os, const struct TransferBalanceReply &reply) {
	os << TransferBalance << "Reply{ .opcode = " << reply.opcode;
	if (reply.opcode == Success) {
		os << ", .from_balance = " << reply.from_balance << ", .to_balance = " << reply.to_balance;
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct SetMatchPriceBounds &cmd) {
	return os << SetMatchPriceBounds << "{ .asset_pair = " << cmd.asset_pair << ", .lower_bound = " << cmd.lower_bound << ", .upper_bound = " << cmd.upper_bound << " }";
}

std::ostream & operator << (std::ostream &os, const struct SetMatchPriceBoundsReply &reply) {
	return os << SetMatchPriceBounds << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct SetNextOrderID &cmd) {
	return os << SetNextOrderID << "{ .next_order_id = " << cmd.next_order_id << " }";
}

std::ostream & operator << (std::ostream &os, const struct SetNextOrderIDReply &reply) {
	return os << SetNextOrderID << "Reply{ .opcode = " << reply.opcode << " }";
}

std::ostream & operator << (std::ostream &os, const struct AdjustBalances &cmd) {
	os << AdjustBalances << "{ .adjustments = [" << std::showbase;
	for (size_t i = 0; i < cmd.adjustments_count; ++i) {
		auto &adj = cmd.adjustments[i];
		os << (i == 0 ? " " : ", ") << "{ .user_id = " << adj.user_id << ", .asset_id = " << std::hex << adj.asset_id << std::dec << ", .delta = " << adj.delta << ", .seq_num = " << adj.seq_num << " }";
	}
	return os << " ], .details = " << std::quoted(std::string_view(reinterpret_cast<const char *>(cmd.details()), cmd.details_size)) << " }";
}

std::ostream & operator << (std::ostream &os, const struct AdjustBalancesReply &reply) {
	return os << AdjustBalances << "Reply{ .opcode = " << reply.opcode << " }";
}


std::ostream & operator << (std::ostream &os, const struct BalanceChanged &notice) {
	return os << BalanceChanged << "{ .user_id = " << notice.user_id << ", .balance = " << notice.balance << " }";
}

std::ostream & operator << (std::ostream &os, const struct BalanceAdjusted &notice) {
	return os << BalanceAdjusted << "{ .user_id = " << notice.user_id << ", .balance = " << notice.balance << ", .seq_num = " << notice.seq_num << " }";
}

std::ostream & operator << (std::ostream &os, const struct OrderOpened &notice) {
	return os << OrderOpened << "{ .user_id = " << notice.user_id << ", .order = " << notice.order << " }";
}

std::ostream & operator << (std::ostream &os, const struct OrderModified &notice) {
	return os << OrderModified << "{ .user_id = " << notice.user_id << ", .order = " << notice.order << " }";
}

static std::ostream & print_base(std::ostream &os, const struct OrdersMatched_v0 &notice) {
	os << OrdersMatched << "{ .bid_user_id = " << notice.bid_user_id << ", .bid_order_id = ";
	if (~notice.bid_order_id) {
		os << notice.bid_order_id;
	}
	else {
		os << '-';
	}
	os << ", .bid_tonce = " << notice.bid_tonce << ", .ask_user_id = " << notice.ask_user_id << ", .ask_order_id = ";
	if (~notice.ask_order_id) {
		os << notice.ask_order_id;
	}
	else {
		os << '-';
	}
	os << ", .ask_tonce = " << notice.ask_tonce << ", .asset_pair = " << notice.asset_pair << ", .quantity = " << notice.quantity << ", .price = " << notice.price << ", .total = " << notice.total << ", .bid_base_fee = " << notice.bid_base_fee << ", .bid_counter_fee = " << notice.bid_counter_fee << ", .ask_base_fee = " << notice.ask_base_fee << ", .ask_counter_fee = " << notice.ask_counter_fee << ", .bid_remaining_quantity = ";
	if (~notice.bid_remaining_quantity) {
		os << notice.bid_remaining_quantity;
	}
	else {
		os << '-';
	}
	os << ", .ask_remaining_quantity = ";
	if (~notice.ask_remaining_quantity) {
		os << notice.ask_remaining_quantity;
	}
	else {
		os << '-';
	}
	return os << ", .time = " << notice.time.time_since_epoch().count();
}

std::ostream & operator << (std::ostream &os, const struct OrdersMatched_v0 &notice) {
	return print_base(os, notice) << " }";
}

std::ostream & operator << (std::ostream &os, const struct OrdersMatched &notice) {
	print_base(os, notice);
	if (notice.bid_flags) {
		os << ", .bid_flags = " << notice.bid_flags;
	}
	if (notice.ask_flags) {
		os << ", .ask_flags = " << notice.ask_flags;
	}
	return os << " }";
}

std::ostream & operator << (std::ostream &os, const struct OrderClosed &notice) {
	return os << OrderClosed << "{ .user_id = " << notice.user_id << ", .order = " << notice.order << ", .time_closed = " << notice.time_closed.time_since_epoch().count() << " }";
}

std::ostream & operator << (std::ostream &os, const struct TickerChanged &notice) {
	return os << TickerChanged << "{ .asset_pair = " << notice.asset_pair << ", .last = " << notice.last << ", .bid = " << notice.bid << ", .ask = " << notice.ask << ", .low_24h = " << notice.low_24h << ", .high_24h = " << notice.high_24h << ", .volume_24h = " << notice.volume_24h << ", .time = " << notice.time.time_since_epoch().count() << " }";
}

std::ostream & operator << (std::ostream &os, const struct UserPublicKeyChanged &notice) {
	return os << UserPublicKeyChanged << "{ .user_id = " << notice.user_id << ", .public_key = [ ... ] }";
}

std::ostream & operator << (std::ostream &os, const struct MatchPriceBoundsChanged &notice) {
	return os << MatchPriceBoundsChanged << "{ .asset_pair = " << notice.asset_pair << ", .lower_bound = " << notice.lower_bound << ", .upper_bound = " << notice.upper_bound << " }";
}

std::ostream & operator << (std::ostream &os, const struct UserTradeVolumeChanged &notice) {
	return os << UserTradeVolumeChanged << "{ .user_id = " << notice.user_id << ", .asset_id = " << std::showbase << std::hex << notice.asset_id << std::dec << ", .volume = " << notice.volume << " }";
}

std::ostream & print(std::ostream &os, const struct BalancesAdjusted &notice, std::string_view details) {
	os << BalancesAdjusted << "{ .adjustments = [" << std::showbase;
	for (size_t i = 0; i < notice.adjustments_count; ++i) {
		auto &adj = notice.adjustments[i];
		os << (i == 0 ? " " : ", ") << "{ .user_id = " << adj.user_id << ", .asset_id = " << std::hex << adj.asset_id << std::dec << ", .balance = " << adj.balance << ", .seq_num = " << adj.seq_num << " }";
	}
	return os << " ], .details = " << std::quoted(details) << " }";
}

std::ostream & operator << (std::ostream &os, const struct BalancesAdjusted &notice) {
	return print(os, notice, { reinterpret_cast<const char *>(notice.details()), notice.details_size });
}


} // namespace core
