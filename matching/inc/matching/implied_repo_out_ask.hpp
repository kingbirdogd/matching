#ifndef MATCHING_INC_MATCHING_IMPLIED_REPO_OUT_ASK_HPP_
#define MATCHING_INC_MATCHING_IMPLIED_REPO_OUT_ASK_HPP_

#include <repo_out_ask_implier.hpp>
#include <matching/implier_base.hpp>

namespace matching
{
	class implied_repo_out_ask : public implier_base, public repo_out_ask_implier
	{
	public:
		implied_repo_out_ask(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e,
				long long pips,
				unsigned long long factor):
			implier(pips),
			implier_base(priority,
					leg1_e,
					leg2_e,
					order::order_side::BUY,
					order::order_side::BUY,
					pips),
					repo_out_ask_implier(pips, factor)
		{
		}
		implied_repo_out_ask(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e):
				implier(0),
				implier_base(priority,
							leg1_e,
							leg2_e,
							order::order_side::BUY,
							order::order_side::BUY,
							0),
							repo_out_ask_implier(0, 1)
		{
		}
		implied_repo_out_ask(const implied_repo_out_ask&) = default;
		implied_repo_out_ask(implied_repo_out_ask&&) = default;
		implied_repo_out_ask& operator= (const implied_repo_out_ask&) = default;
		implied_repo_out_ask& operator= (implied_repo_out_ask&&) = default;
		virtual ~implied_repo_out_ask() = default;
	};
}





#endif /* MATCHING_INC_MATCHING_IMPLIED_REPO_OUT_ASK_HPP_ */
