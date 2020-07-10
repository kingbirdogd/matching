#ifndef IMPLIER_INC_MINUS_BID_IMPLIER_HPP_
#define IMPLIER_INC_MINUS_BID_IMPLIER_HPP_

#include <implier.hpp>
#include <math.hpp>

class minus_bid_implier : public virtual implier
{
public:
	minus_bid_implier(long long pips):
		implier(pips)
	{
	}
	virtual ~minus_bid_implier() = default;
	virtual long long matchd_price(long long leg1_price, long long leg2_price, long long mini_tick)
	{
		return round_down(leg1_price - leg2_price - handle_pips(leg1_price, _pips) - handle_pips(leg2_price, _pips), mini_tick);
	}
};



#endif /* IMPLIER_INC_MINUS_BID_IMPLIER_HPP_ */
