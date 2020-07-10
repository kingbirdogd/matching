#ifndef IMPLIER_INC_MINUS_ASK_IMPLIER_HPP_
#define IMPLIER_INC_MINUS_ASK_IMPLIER_HPP_

#include <implier.hpp>
#include <math.hpp>

class minus_ask_implier : public virtual implier
{
public:
	minus_ask_implier(long long pips):
		implier(pips)
	{
	}
	virtual ~minus_ask_implier() = default;
	virtual long long matchd_price(long long leg1_price, long long leg2_price, long long mini_tick)
	{
		return round_up(leg1_price - leg2_price + handle_pips(leg1_price, _pips) + handle_pips(leg2_price, _pips), mini_tick);
	}
};



#endif /* IMPLIER_INC_MINUS_ASK_IMPLIER_HPP_ */
