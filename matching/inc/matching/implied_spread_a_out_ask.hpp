#ifndef MATCHING_INC_MATCHING_IMPLIED_SPREAD_A_OUT_ASK_HPP_
#define MATCHING_INC_MATCHING_IMPLIED_SPREAD_A_OUT_ASK_HPP_

#include <add_ask_implier.hpp>
#include <matching/implier_base.hpp>

namespace matching
{
	class implied_spread_a_out_ask : public implier_base, public add_ask_implier
	{
	public:
		implied_spread_a_out_ask(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e,
				long long pips):
			implier(pips),
			implier_base(priority,
					leg1_e,
					leg2_e,
					order::order_side::BUY,
					order::order_side::BUY,
					pips),
					add_ask_implier(pips)
		{
		}
		implied_spread_a_out_ask(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e):
				implier(0),
				implier_base(priority,
							leg1_e,
							leg2_e,
							order::order_side::BUY,
							order::order_side::BUY,
							0),
				add_ask_implier(0)
		{
		}
		implied_spread_a_out_ask(const implied_spread_a_out_ask&) = default;
		implied_spread_a_out_ask(implied_spread_a_out_ask&&) = default;
		implied_spread_a_out_ask& operator= (const implied_spread_a_out_ask&) = default;
		implied_spread_a_out_ask& operator= (implied_spread_a_out_ask&&) = default;
		virtual ~implied_spread_a_out_ask() = default;
	};
}





#endif /* MATCHING_INC_MATCHING_IMPLIED_SPREAD_A_OUT_ASK_HPP_ */
