#ifndef MATCHING_INC_ORDER_HPP_
#define MATCHING_INC_ORDER_HPP_

#include <limits>
#include <vector>

namespace matching
{
	struct order
	{
	public:
		const static long long MARKET_PRICE = std::numeric_limits<long long>::max();
	public:
		class less
		{
		public:
			constexpr bool operator()(const long long& l, const long long& r) const
			{
				return ((MARKET_PRICE == r) ? true : (l < r));
			}
		};
		class greater
		{
		public:
			constexpr bool operator()(const long long& l, const long long& r) const
			{
				return ((MARKET_PRICE == r) ? true : (l > r));
			}
		};
	public:
		enum order_side : unsigned char
		{
			BUY = 0x00,
			SELL = 0x01,
			BUY_STOP = 0x02,
			SELL_STOP = 0x03,
			BUY_SELL_STOP = 0x04
		};
		enum order_type : unsigned char
		{
			LIMITED = 0x00,
			MARKET = 0x01
		};
		enum order_action_type : unsigned char
		{
			NEW = 0x00,
			AMEND = 0x01,
			CANCEL = 0x02
		};
		enum order_time_condition : unsigned char
		{
			GTC = 0x00,
			IOC = 0x01,
			FOK = 0x02,
			MAKER_ONLY = 0x03,
			MAKER_ONLY_REPRICE = 0x04
		};
		enum order_status_type : unsigned char
		{
			OPEN = 0x00,
			PARTIAL_FILL = 0x01,
			FILLED = 0x02,
			CANCELED_BY_USER = 0x03,
			CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED = 0x04,
			CANCELED_BY_MARKET_ORDER_NOTHING_MATCH = 0x05,
			CANCELED_ALL_BY_IOC = 0x06,
			CANCELED_PARTIAL_BY_IOC = 0x07,
			CANCELED_BY_FOK = 0x08,
			CANCELED_BY_MAKER_ONLY = 0x09,
			REJECT_CANCEL_ORDER_ID_NOT_FOUND = 0xA,
			REJECT_AMEND_ORDER_ID_NOT_FOUND = 0x0B,
			REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY = 0x0C,
			REJECT_BUY_STOP_TRIGGER_LESS_THAN_STOP_LIMITED = 0x0D,
			REJECT_BUY_STOP_NO_BEST_ASK = 0x0E,
			REJECT_BUY_STOP_TRIGGER_LESS_THAN_BEST_ASK = 0x0F,
			REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED = 0x10,
			REJECT_SELL_STOP_NO_BEST_BID = 0x11,
			REJECT_SELL_STOP_TRIGGER_LESS_THAN_BEST_BID = 0x12,
			REJECT_BUY_SELL_STOP_TRIGGER_CROSS = 0x13,
			REJECT_UNKNOW_ORDER_ACTION = 0x14,
			REJECT_QUANTITY_ZERO = 0x15,
			REJECT_LIMITED_PRICE_ERROR = 0x16
		};
		enum order_matched_type : unsigned char
		{
			MAKER = 0x00,
			TAKER = 0x01
		};
	public:
		long long price;
		unsigned long long quantity;
		unsigned long long display_quantity;
		unsigned long long remain_quantity;
		long long last_match_price;
		unsigned long long last_match_quantity;
		unsigned long long order_id;
		unsigned long long client_order_id;
		unsigned long long last_matched_order_id;
		unsigned long long last_matched_order_id2;
		unsigned long long matched_id;
		long long buy_stop_trigger_price;
		long long buy_stop_limited_price;
		long long sell_stop_trigger_price;
		long long sell_stop_limited_price;
		order_side side;
		order_type type;
		order_action_type order_action;
		order_time_condition time_condition;
		order_status_type order_state;
		order_matched_type matched_type;
	private:
		inline unsigned int _get_flag_value() const
		{
			return (*static_cast<const unsigned int*>(static_cast<const void*>(&side)));
		}
	public:
		order():
			price(MARKET_PRICE),
			quantity(0),
			display_quantity(0),
			remain_quantity(0),
			last_match_price(0),
			last_match_quantity(0),
			order_id(0),
			client_order_id(0),
			last_matched_order_id(0),
			last_matched_order_id2(2),
			matched_id(0),
			buy_stop_trigger_price(0),
			buy_stop_limited_price(MARKET_PRICE),
			sell_stop_trigger_price(0),
			sell_stop_limited_price(MARKET_PRICE),
			side(order_side::BUY),
			type(order_type::LIMITED),
			order_action(order_action_type::NEW),
			time_condition(order_time_condition::GTC),
			order_state(order_status_type::OPEN),
			matched_type(order_matched_type::MAKER)
		{
		}
		~order() = default;
		void update_display()
		{
			display_quantity = display_quantity < remain_quantity ? display_quantity : remain_quantity;
		}
		inline static bool can_amend(const order& o1, const order& o2)
		{
			if (o1._get_flag_value() != o2._get_flag_value())
			{
				return false;
			}
			if (order_side::BUY == o1.side || order_side::SELL == o1.side)
			{
				if (o1.price != o2.price)
					return false;
			}
			else if (order_side::BUY_STOP == o1.side)
			{
				if (o1.buy_stop_limited_price != o2.buy_stop_limited_price)
					return false;
				if (o1.buy_stop_trigger_price != o2.buy_stop_trigger_price)
					return false;
			}
			else if (order_side::SELL_STOP == o1.side)
			{
				if (o1.sell_stop_limited_price != o2.sell_stop_limited_price)
					return false;
				if (o1.sell_stop_trigger_price != o2.sell_stop_trigger_price)
					return false;
			}
			else
			{
				if (o1.buy_stop_limited_price != o2.buy_stop_limited_price)
					return false;
				if (o1.buy_stop_trigger_price != o2.buy_stop_trigger_price)
					return false;
				if (o1.sell_stop_limited_price != o2.sell_stop_limited_price)
					return false;
				if (o1.sell_stop_trigger_price != o2.sell_stop_trigger_price)
					return false;
			}
			if (o1.quantity > o2.remain_quantity)
				return false;
			return true;
		}
	public:
		struct matched_record
		{
			long long last_match_price;
			unsigned long long last_match_quantity;
			unsigned long long matched_order_id1;
			unsigned long long matched_order_id2;
			matched_record():
				last_match_price(0),
				last_match_quantity(0),
				matched_order_id1(0),
				matched_order_id2(0)
			{
			}
			~matched_record() = default;
		};
		struct implied_matche_record
		{
			order* odr;
			unsigned long long remain_quantity;
			std::vector<matched_record> records;
			implied_matche_record():
				odr(nullptr),
				remain_quantity(0)
			{
			}
			implied_matche_record(order* o):
				odr(o),
				remain_quantity(odr->remain_quantity),
				records()
			{
			}
			~implied_matche_record() = default;
		};
		using implied_matche_records = std::vector<implied_matche_record>;
		struct implied_matche_order_top_result
		{
			implied_matche_records records;
			unsigned long long total_quantity;
			implied_matche_order_top_result():
				records(),
				total_quantity(0)
			{
			}
			~implied_matche_order_top_result() = default;
		};
	};
}



#endif /* MATCHING_INC_ORDER_HPP_ */
