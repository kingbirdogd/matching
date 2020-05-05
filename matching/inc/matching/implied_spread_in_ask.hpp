#ifndef MATCHING_INC_MATCHING_IMPLIED_SPREAD_IN_ASK_HPP_
#define MATCHING_INC_MATCHING_IMPLIED_SPREAD_IN_ASK_HPP_

#include <matching/implier_base.hpp>

namespace matching
{
	class implied_spread_in_ask : public implier_base
	{
	public:
		implied_spread_in_ask(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e,
				long long pips):
			implier_base(priority,
					leg1_e,
					leg2_e,
					order::order_side::BUY,
					order::order_side::SELL,
					pips)
		{
		}
		implied_spread_in_ask(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e):
				implier_base(priority,
							leg1_e,
							leg2_e,
							order::order_side::BUY,
							order::order_side::SELL,
							0)
		{
		}
		implied_spread_in_ask(const implied_spread_in_ask&) = default;
		implied_spread_in_ask(implied_spread_in_ask&&) = default;
		implied_spread_in_ask& operator= (const implied_spread_in_ask&) = default;
		implied_spread_in_ask& operator= (implied_spread_in_ask&&) = default;
		virtual ~implied_spread_in_ask() = default;
		virtual long long matchd_price(long long leg1_price, long long leg2_price, unsigned long long mini_tick)
		{
			return engine::round_up(leg1_price - leg2_price + 2 * _pips, mini_tick);
		}
	};
}



#endif /* MATCHING_INC_MATCHING_IMPLIED_SPREAD_IN_ASK_HPP_ */
