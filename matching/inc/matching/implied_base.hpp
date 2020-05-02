#ifndef MATCHING_ORDER_INC_MATCHING_IMPLIED_BASE_HPP_
#define MATCHING_ORDER_INC_MATCHING_IMPLIED_BASE_HPP_

namespace matching
{
	struct implied_order;
	class engine;
	class implied_base
	{
	private:
		friend engine;
	protected:
		void _place_order(engine* e, implied_order& o);
	public:
		implied_base() = default;
		virtual ~implied_base() = default;
		virtual void handle_bid_add_quantity(engine* e, long long price, unsigned long long quantity) = 0;
		virtual void handle_bid_sub_quantity(engine* e, long long price, unsigned long long quantity) = 0;
		virtual void handle_ask_add_quantity(engine* e, long long price, unsigned long long quantity) = 0;
		virtual void handle_ask_sub_quantity(engine* e, long long price, unsigned long long quantity) = 0;
		virtual void handle_bid_implied_cross(engine* e, unsigned long long quantity) = 0;
		virtual void handle_ask_implied_cross(engine* e, unsigned long long quantity) = 0;
	};
};



#endif /* MATCHING_ORDER_INC_MATCHING_IMPLIED_BASE_HPP_ */
