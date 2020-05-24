#ifndef MATCHING_INC_MATCHING_IMPLIED_SPREAD_A_OUT_BID_HPP_
#define MATCHING_INC_MATCHING_IMPLIED_SPREAD_A_OUT_BID_HPP_


#include <add_bid_implier.hpp>
#include <matching/implier_base.hpp>

namespace matching
{
	class implied_spread_a_out_bid : public implier_base, public add_bid_implier
	{
	public:
		implied_spread_a_out_bid(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e,
				long long pips):
			implier(pips),
			implier_base(priority,
					leg1_e,
					leg2_e,
					order::order_side::SELL,
					order::order_side::SELL,
					pips),
					add_bid_implier(pips)
		{
		}
		implied_spread_a_out_bid(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e):
				implier(0),
				implier_base(priority,
							leg1_e,
							leg2_e,
							order::order_side::SELL,
							order::order_side::SELL,
							0),
				add_bid_implier(0)
		{
		}
		implied_spread_a_out_bid(const implied_spread_a_out_bid&) = default;
		implied_spread_a_out_bid(implied_spread_a_out_bid&&) = default;
		implied_spread_a_out_bid& operator= (const implied_spread_a_out_bid&) = default;
		implied_spread_a_out_bid& operator= (implied_spread_a_out_bid&&) = default;
		virtual ~implied_spread_a_out_bid() = default;
	};
}






#endif /* MATCHING_INC_MATCHING_IMPLIED_SPREAD_A_OUT_BID_HPP_ */
