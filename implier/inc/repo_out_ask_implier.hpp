#ifndef IMPLIER_INC_REPO_OUT_ASK_IMPLIER_HPP_
#define IMPLIER_INC_REPO_OUT_ASK_IMPLIER_HPP_

#include <implier.hpp>
#include <math.hpp>

class repo_out_ask_implier : public virtual implier
{
private:
	unsigned long long factor_;
public:
	repo_out_ask_implier(long long pips, unsigned long long factor):
		implier(pips),
		factor_(factor)
	{
	}
	virtual ~repo_out_ask_implier() = default;
	virtual long long matchd_price(long long leg1_price, long long leg2_price, long long mini_tick)
	{
		return round_up(round_up(leg1_price * (double(factor_) + leg2_price) / factor_, 1) + handle_pips(leg1_price, _pips),mini_tick);
	}
};

#endif /* IMPLIER_INC_REPO_OUT_ASK_IMPLIER_HPP_ */
