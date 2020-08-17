#ifndef MATCHING_ORDER_INC_MATCHING_ORDER_HANDLER_HPP_
#define MATCHING_ORDER_INC_MATCHING_ORDER_HANDLER_HPP_

#include <functional>
#include <cstring>
#include <matching/order.hpp>
#include <iostream>
namespace matching
{
	class order_handler
	{
	private:
		using handler = std::function<void(const order&)>;
	private:
		char buff_[sizeof(order)];
		std::size_t rest_;
		handler h_;
	public:
		order_handler(handler&& h):
			buff_{},
			rest_(0),
			h_(std::move(h))
		{
		}
		order_handler(order_handler&& h):
			buff_{},
			rest_(0),
			h_(std::move(h.h_))
		{
		}
		order_handler& operator= (order_handler&& h)
		{
			std::memcpy(buff_, h.buff_, h.rest_);
			rest_ = h.rest_;
			h_ = std::move(h.h_);
			return *this;
		}
		void handle(const char* ptr, std::size_t size)
		{
			if (0 != rest_)
			{
				auto need = sizeof(order) - rest_;
				auto cp_size = need < size ? need : size;
				std::memcpy(buff_ + rest_, ptr, cp_size);
				rest_ += cp_size;
				if (sizeof(order) != rest_)
					return;
				h_(*static_cast<const order*>(static_cast<const void*>(buff_)));
				ptr += cp_size;
				size -= cp_size;
				rest_ = 0;
			}
			while (size >= sizeof(order))
			{
				h_(*static_cast<const order*>(static_cast<const void*>(ptr)));
				ptr += sizeof(order);
				size -= sizeof(order);
			}
			if (0 != size)
			{
				std::memcpy(buff_, ptr, size);
				rest_ = size;
			}
		}
		order_handler() = delete;
		order_handler(const order_handler& h) = delete;
		order_handler operator= (const order_handler& h) = delete;
	};
}



#endif /* MATCHING_ORDER_INC_MATCHING_ORDER_HANDLER_HPP_ */
