#ifndef MATCHING_INC_MATCHING_IMPLIER_BASE_HPP_
#define MATCHING_INC_MATCHING_IMPLIER_BASE_HPP_

#include <matching/engine.hpp>

namespace matching
{
	class implier_base : public engine::implier_base
	{
	public:
		implier_base() = delete;
		implier_base(unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e,
				order::order_side leg1_side,
				order::order_side leg2_side,
				unsigned long long pips):
			engine::implier_base(priority,
					leg1_e,
					leg2_e,
					leg1_side,
					leg2_side,
					pips)
		{
		}
		implier_base(const implier_base&) = default;
		implier_base(implier_base&&) = default;
		implier_base& operator= (const implier_base&) = default;
		implier_base& operator= (implier_base&&) = default;
		virtual ~implier_base() = default;
	};
}



#endif /* MATCHING_INC_MATCHING_IMPLIER_BASE_HPP_ */
