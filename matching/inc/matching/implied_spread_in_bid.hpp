#ifndef MATCHING_INC_MATCHING_IMPLIED_SPREAD_IN_BID_HPP_
#define MATCHING_INC_MATCHING_IMPLIED_SPREAD_IN_BID_HPP_

#include <matching/implier_base.hpp>

namespace matching
{
	class implied_spread_in_bid : public implier_base
	{
	public:
		implied_spread_in_bid(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e,
				long long pips):
			implier_base(priority,
					leg1_e,
					leg2_e,
					matching::order::order_side::SELL,
					matching::order::order_side::BUY,
					pips)
		{
		}
		implied_spread_in_bid(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e):
				implier_base(priority,
							leg1_e,
							leg2_e,
							matching::order::order_side::SELL,
							matching::order::order_side::BUY,
							0)
		{
		}
		implied_spread_in_bid(const implied_spread_in_bid&) = default;
		implied_spread_in_bid(implied_spread_in_bid&&) = default;
		implied_spread_in_bid& operator= (const implied_spread_in_bid&) = default;
		implied_spread_in_bid& operator= (implied_spread_in_bid&&) = default;
		virtual ~implied_spread_in_bid() = default;
		virtual long long matchd_price(long long leg1_price, long long leg2_price, unsigned long long mini_tick)
		{
			return engine::round_up(leg1_price - leg2_price + 2 * _pips, mini_tick);
		}
	};



#endif /* MATCHING_INC_MATCHING_IMPLIED_SPREAD_IN_BID_HPP_ */
