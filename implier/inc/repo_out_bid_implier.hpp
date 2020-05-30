#ifndef IMPLIER_INC_REPO_OUT_BID_IMPLIER_HPP_
#define IMPLIER_INC_REPO_OUT_BID_IMPLIER_HPP_


#include <implier.hpp>
#include <math.hpp>

class repo_out_bid_implier : public virtual implier
{
public:
	repo_out_bid_implier(unsigned long long pips):
		implier(pips)
	{
	}
	virtual ~repo_out_bid_implier() = default;
	virtual long long matchd_price(long long leg1_price, long long leg2_price, unsigned long long mini_tick)
	{
		return round_down(leg1_price * (1 + leg2_price) -  2 * _pips, mini_tick);
	}
};


#endif /* IMPLIER_INC_REPO_OUT_BID_IMPLIER_HPP_ */
