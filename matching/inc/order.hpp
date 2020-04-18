#ifndef MATCHING_INC_ORDER_HPP_
#define MATCHING_INC_ORDER_HPP_

#include <limits>

namespace matching
{
	struct order
	{
	public:
		static long long MARKET_PRICE = std::numeric_limits<long long>::max();
	public:
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
			MAKER_ONLY = 0x03
		};
		enum oder_engine_type : unsigned char
		{
			NORMAL = 0x00,
			IMPLIED = 0x01
		};
		enum order_stop_type : unsigned char
		{
			NONE = 0x00,
			CUT_LOST = 0x01,
			TAKE_PROFIT = 0x02,
			CUT_LOST_TAKE_PROFIT = 0x03,
			CUT_LOST_TAKE_PROFIT_WITHOUT_POSITION = 0x04 //directory create two related order
		};
		enum order_stop_condition_type : unsigned char
		{
			ABSOLUTE = 0x00,
			DELTA = 0x01,
			PERCENTAGE = 0x02 //8 digit
		};
		enum order_status_type : unsigned char
		{
			OPEN = 0x00,
			PARTIAL_FILL = 0x01,
			FILLED = 0x02,
			CANCELED_BY_USER = 0x03,
			CANCELED_ALL_BY_IOC = 0x04,
			CANCELED_PARTIAL_BY_IOC = 0x05,
			CANCELED_BY_FOK = 0x06,
			CANCELED_BY_MAKER_ONLY = 0x07,
			CANCELED_BY_RELATED_ORDER = 0x08,
			REJECT_CUT_LOST_VALUE_CROSS_BEST_PRICE = 0x09,
			REJECT_TAKE_PROFIT_VALUE_CROSS_BEST_PRICE = 0x0A,
			REJECT_CUT_LOST_TAKE_PROFIT_VALUE_CROSS = 0x0B
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
		long long last_match_quantity;
		unsigned long long order_id;
		unsigned long long client_order_id;
		unsigned long long last_matched_order_id;
		unsigned long long related_order_id;
		long long cut_lost_value; //can be price, delta, or percentage
		long long take_profit_value; //can be price, delta, or percentage
		order_action_type order_action;
		order_stop_type stop_type;
		order_stop_condition_type cut_lost_condition;
		order_stop_condition_type take_profit_condition;
		oder_engine_type engine_type;
		order_time_condition time_condition;
		order_status_type order_state;
		order_matched_type matched_type;
	};
}



#endif /* MATCHING_INC_ORDER_HPP_ */
