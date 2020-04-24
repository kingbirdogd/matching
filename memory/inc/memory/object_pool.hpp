#ifndef MEMORY_INC_MEMORY_OBJECT_POOL_HPP_
#define MEMORY_INC_MEMORY_OBJECT_POOL_HPP_

#include <cstddef>

namespace memory
{
	template <typename TYPE, std::size_t SIZE>
	class object_pool
	{
	private:
		unsigned char buff_[SIZE * sizeof(TYPE)];
		TYPE* stack_[SIZE];
		std::size_t top_;
	public:
		object_pool():
			buff_{},
			stack_{},
			top_(SIZE)
		{
			auto ptr = static_cast<TYPE*>(static_cast<void*>(buff_));
			for (std::size_t i = 0; i < SIZE; ++i)
				stack_[i] = ptr++;
		}
		~object_pool() = default;
		template<typename ...TArgs>
		TYPE* alloc(TArgs&&... args)
		{
			if (0 == top_)
				return nullptr;
			auto ptr = stack_[--top_];
			new (ptr) TYPE(args...);
			return ptr;
		}
		bool free(TYPE* ptr)
		{
			if (SIZE == top_)
				return false;
			ptr->~TYPE();
			stack_[top_++] = ptr;
			return true;
		}
	};
};



#endif /* MEMORY_INC_MEMORY_OBJECT_POOL_HPP_ */
