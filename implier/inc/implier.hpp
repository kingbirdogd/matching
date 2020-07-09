#ifndef IMPLIER_INC_IMPLIER_HPP_
#define IMPLIER_INC_IMPLIER_HPP_

class implier
{
protected:
	long long _pips;
public:
	implier(long long pips):
		_pips(pips)
	{
	}
	virtual ~implier() = default;
	virtual long long matchd_price(long long leg1_price, long long leg2_price, long long mini_tick) = 0;
};



#endif /* IMPLIER_INC_IMPLIER_HPP_ */
