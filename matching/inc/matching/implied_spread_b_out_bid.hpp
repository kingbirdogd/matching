#ifndef MATCHING_INC_MATCHING_IMPLIED_SPREAD_B_OUT_BID_HPP_
#define MATCHING_INC_MATCHING_IMPLIED_SPREAD_B_OUT_BID_HPP_



#include <matching/implier_base.hpp>

namespace matching
{
	class implied_spread_b_out_bid : public implier_base
	{
	public:
		implied_spread_b_out_bid(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e,
				long long pips):
			implier_base(priority,
					leg1_e,
					leg2_e,
					order::order_side::SELL,
					order::order_side::BUY,
					pips)
		{
		}
		implied_spread_b_out_bid(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e):
				implier_base(priority,
							leg1_e,
							leg2_e,
							order::order_side::SELL,
							order::order_side::BUY,
							0)
		{
		}
		implied_spread_b_out_bid(const implied_spread_b_out_bid&) = default;
		implied_spread_b_out_bid(implied_spread_b_out_bid&&) = default;
		implied_spread_b_out_bid& operator= (const implied_spread_b_out_bid&) = default;
		implied_spread_b_out_bid& operator= (implied_spread_b_out_bid&&) = default;
		virtual ~implied_spread_b_out_bid() = default;
		virtual long long matchd_price(long long leg1_price, long long leg2_price, unsigned long long mini_tick)
		{
			return engine::round_down(leg1_price - leg2_price + 2 * _pips, mini_tick);
		}
	};
}




#endif /* MATCHING_INC_MATCHING_IMPLIED_SPREAD_B_OUT_BID_HPP_ */
