#ifndef MATCHING_INC_MATCHING_IMPLIED_REPO_OUT_BID_HPP_
#define MATCHING_INC_MATCHING_IMPLIED_REPO_OUT_BID_HPP_

#include <repo_out_bid_implier.hpp>
#include <matching/implier_base.hpp>

namespace matching
{
	class implied_repo_out_bid : public implier_base, public repo_out_bid_implier
	{
	public:
		implied_repo_out_bid(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e,
				long long pips,
				unsigned long long factor):
			implier(pips),
			implier_base(priority,
					leg1_e,
					leg2_e,
					order::order_side::SELL,
					order::order_side::SELL,
					pips),
					repo_out_bid_implier(pips, factor)
		{
		}
		implied_repo_out_bid(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e):
				implier(0),
				implier_base(priority,
							leg1_e,
							leg2_e,
							order::order_side::SELL,
							order::order_side::SELL,
							0),
				repo_out_bid_implier(0, 1)
		{
		}
		implied_repo_out_bid(const implied_repo_out_bid&) = default;
		implied_repo_out_bid(implied_repo_out_bid&&) = default;
		implied_repo_out_bid& operator= (const implied_repo_out_bid&) = default;
		implied_repo_out_bid& operator= (implied_repo_out_bid&&) = default;
		virtual ~implied_repo_out_bid() = default;
	};
}




#endif /* MATCHING_INC_MATCHING_IMPLIED_REPO_OUT_BID_HPP_ */
