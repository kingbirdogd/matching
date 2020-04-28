#ifndef MATCHING_INC_MATCHING_IMPLIED_ORDER_HPP_
#define MATCHING_INC_MATCHING_IMPLIED_ORDER_HPP_

#include <matching/implied_base.hpp>
#include <matching/order.hpp>

namespace matching
{
	struct implied_order : public order
	{
		implied_base* implied_ptr;
		implied_order():
			order(),
			implied_ptr(nullptr)
		{
		}
		implied_order(const implied_order&) = default;
		implied_order(implied_order&&) = default;
		implied_order& operator= (const implied_order&) = default;
		implied_order& operator= (implied_order&&) = default;
		implied_order(const order& o):
			order(o),
			implied_ptr(nullptr)
		{
		}
		implied_order(order&& o):
			order(std::move(o)),
			implied_ptr(nullptr)
		{
		}
		implied_order& operator= (const order& o)
		{
			auto& self = (order&)(*this);
			self = o;
			implied_ptr = nullptr;
			return *this;
		}
		implied_order& operator= (order&& o)
		{
			auto& self = (order&)(*this);
			self = std::move(o);
			implied_ptr = nullptr;
			return *this;
		}
		~implied_order() = default;
	};
}



#endif
