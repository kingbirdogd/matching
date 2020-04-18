#ifndef MATCHING_INC_ENGINE_HPP_
#define MATCHING_INC_ENGINE_HPP_

#include <map>
#include <unordered_map>
#include <atomic>
#include <functional>
#include <order.hpp>

namespace matching
{
	class engine
	{
	private:
		static std::atomic<unsigned long long> _id;
	private:
		inline static unsigned long long get_id()
		{
			return _id.fetch_add(1, std::memory_order_relaxed);
		}
	private:
		using search_order_map = std::unordered_map<unsigned long long, order*>;
		using id_order_map = std::map<unsigned long long, order>;
		using type_order_map = std::map<order::oder_engine_type, id_order_map>;
		using bid_book_type = std::map<long long, type_order_map, std::greater<long long>>;
		using ask_book_type = std::map<long long, type_order_map, std::less<long long>>;
		using callback_type = std::function<void(const order&)>;
	private:
		bid_book_type _bid_book;
		ask_book_type _ask_book;
		callback_type _callback;
	public:
		engine(callback_type&& callback);
		~engine();
	};
}




#endif /* MATCHING_INC_ENGINE_HPP_ */
