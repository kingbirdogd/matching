#ifndef MATCHING_INC_MATCHING_IMPLIER_BASE_HPP_
#define MATCHING_INC_MATCHING_IMPLIER_BASE_HPP_

#include <matching/engine.hpp>

namespace matching
{
	class implier_base : public engine::implier_base
	{
	protected:
		long long _pips;
	public:
		implier_base(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e,
				order::order_side leg1_side,
				order::order_side leg2_side,
				long long pips):
			engine::implier_base(priority,
					leg1_e,
					leg2_e,
					leg1_side,
					leg2_side),
			_pips(pips)
		{
		}
		implier_base(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e,
				order::order_side leg1_side,
				order::order_side leg2_side):
				engine::implier_base(priority,
							leg1_e,
							leg2_e,
							leg1_side,
							leg2_side),
			_pips(0)
		{
		}
		implier_base(const implier_base&) = default;
		implier_base(implier_base&&) = default;
		implier_base& operator= (const implier_base&) = default;
		implier_base& operator= (implier_base&&) = default;
		virtual ~implier_base() = default;
		virtual long long matchd_price(long long leg1_price, long long leg2_price, unsigned long long mini_tick) = 0;
	};
};



#endif /* MATCHING_INC_MATCHING_IMPLIER_BASE_HPP_ */
