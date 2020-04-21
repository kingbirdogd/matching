#ifndef MATCHING_INC_ENGINE_HPP_
#define MATCHING_INC_ENGINE_HPP_

#include <map>
#include <unordered_map>
#include <atomic>
#include <functional>
#include <algorithm>
#include <matching/order.hpp>

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
		using stop_book_type = std::unordered_map<unsigned long long, order>;
		using id_stop_ordermap = std::map<unsigned long long, order*>;
		using bid_stop_book_type = std::map<long long, id_stop_ordermap, std::greater<long long>>;
		using ask_stop_book_type = std::map<long long, id_stop_ordermap, std::less<long long>>;
	private:
		search_order_map _odr_map;
		bid_book_type _bid_book;
		ask_book_type _ask_book;
		stop_book_type _stop_book;
		bid_stop_book_type _bid_stop_book;
		ask_stop_book_type _ask_stop_book;
		callback_type _callback;
	public:
		engine(callback_type&& callback);
		~engine();
		void handle(order& o);
		inline void recovery(callback_type&& callback) const
		{
			for (const auto& item : _odr_map)
			{
				callback(*item.second);
			}
		}
	private:
		template <typename BookType>
		static void erase_from_book(BookType& book, order& odr)
		{
			auto ori_it_price = book.find(odr.price);
			auto ori_it_type = ori_it_price->second.find(odr.engine_type);
			ori_it_type->second.erase(odr.order_id);
			if (ori_it_type->second.empty())
			{
				ori_it_price->second.erase(ori_it_type);
				if (ori_it_price->second.empty())
				{
					book.erase(ori_it_price);
				}
			}
		}

		//get the order keep flag
		inline static unsigned long long get_flag_value(const order& odr)
		{
			return (*static_cast<const unsigned int*>(static_cast<const void*>(&odr.side)));
		}

		template <typename BookSelf, typename BookCross>
		inline void handle_normal(BookSelf& self, BookCross& cross, order& o)
		{
			o.remain_quantity = o.quantity;
			typename BookCross::key_compare cross_cmp;
			if (order::MARKET_PRICE == o.price && cross.empty())
			{
				o.order_state = order::order_status_type::CANCELED_BY_MARKET_ORDER_NOTHING_MATCH;
				_callback(o);
				return;
			}
			bool stop = false;
			for (auto it = cross.begin(); it != cross.end();)
			{
				auto matched_price = it->first;
				if (!cross_cmp(o.price, matched_price) || order::MARKET_PRICE == o.price)
				{
					if (order::order_time_condition::MAKER_ONLY == o.time_condition)
					{
						o.order_state = order::order_status_type::CANCELED_BY_MAKER_ONLY;
						_callback(o);
						return;
					}
					auto m2 = it->second;
					for (auto it2 = m2.begin(); it2 != m2.end();)
					{
						auto m3 = it2->second;
						for (auto it3 = m3.begin(); it3 != m3.end();)
						{
							auto& o2 = it3->second;
							auto matched_quantity = std::min(o.remain_quantity, o2.remain_quantity);
							auto matched_id = get_id();
							o.matched_id = matched_id;
							o2.matched_id = matched_id;
							o.remain_quantity -= matched_quantity;
							o2.remain_quantity -= matched_quantity;
							o.last_match_price = matched_price;
							o.last_match_quantity = matched_quantity;
							o2.last_match_price = matched_price;
							o2.last_match_quantity = matched_quantity;
							o.last_matched_order_id = o2.order_id;
							o2.last_matched_order_id = o.order_id;
							if (0 == o.remain_quantity)
							{
								o.order_state = order::order_status_type::FILLED;
							}
							else
							{
								o.order_state = order::order_status_type::PARTIAL_FILL;
							}
							if (0 == o2.remain_quantity)
							{
								o2.order_state = order::order_status_type::FILLED;
							}
							else
							{
								o2.order_state = order::order_status_type::PARTIAL_FILL;
							}
							o.matched_type = order::order_matched_type::TAKER;
							o2.matched_type = order::order_matched_type::MAKER;
							_callback(o);
							_callback(o2);
							if (0 != o2.remain_quantity)
							{
								++it3;
							}
							else
							{
								it3 = m3.erase(it3);
								_odr_map.erase(o2.order_id);
							}
							if (0 == o.remain_quantity)
							{
								stop = true;
								break;
							}
						}
						if (!m3.empty())
						{
							++it2;
						}
						else
						{
							it2 = m2.erase(it2);
						}
						if (stop)
							break;
					}
					if (!m2.empty())
					{
						++it;
					}
					else
					{
						it = cross.erase(it);
					}
					if (stop)
						break;
				}
				else
				{
					++it;
				}
			}
			if (o.remain_quantity != 0)
			{
				if (order::order_time_condition::IOC == o.time_condition)
				{
					if (0 != o.remain_quantity)
					{
						if (o.remain_quantity == o.quantity)
							o.order_state = order::order_status_type::CANCELED_ALL_BY_IOC;
						else
							o.order_state = order::order_status_type::CANCELED_PARTIAL_BY_IOC;
						_callback(o);
					}
					return;
				}
				if (o.remain_quantity == o.quantity)
					_callback(o);
				self[o.price][o.engine_type][o.order_id] = o;
				_odr_map[o.order_id] = &self[o.price][o.engine_type][o.order_id];
			}
		}

		inline void handle_new(order& o)
		{
			o.order_id = get_id();
			if (o.display_quantity > o.quantity)
			{
				o.order_state = order::order_status_type::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY;
				_callback(o);
				return;
			}
			if (order::order_side::BUY == o.side)
			{
				handle_normal(_bid_book, _ask_book, o);
			}
			else if (order::order_side::SELL == o.side)
			{
				handle_normal(_ask_book, _bid_book, o);
			}

		}

		inline void handle_cancel(order& o)
		{
			auto it = _odr_map.find(o.order_id);
			if (_odr_map.end() == it)
			{
				o.order_state = order::order_status_type::REJECT_CANCEL_ORDER_ID_NOT_FOUND;
				_callback(o);
				return;
			}
			auto& ori_odr = *(it->second);
			if (order::oder_engine_type::IMPLIED == ori_odr.engine_type)
			{
				o.order_state = order::order_status_type::REJECT_CANCEL_ORDER_ID_NOT_FOUND;
				_callback(o);
				return;
			}
			ori_odr.client_order_id = o.client_order_id;
			ori_odr.order_state = order::order_status_type::CANCELED_BY_USER;
			_callback(ori_odr);
			if (order::order_side::BUY == ori_odr.side)
			{
				erase_from_book(_bid_book, ori_odr);
			}
			else
			{
				erase_from_book(_ask_book, ori_odr);
			}
			_odr_map.erase(it);
		}

		//can keep the priority if just reduce quantity
		inline void handle_amend(order& o)
		{
			if (o.display_quantity > o.quantity)
			{
				o.order_state = order::order_status_type::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY;
				_callback(o);
				return;
			}
			auto it = _odr_map.find(o.order_id);
			if (_odr_map.end() == it)
			{
				o.order_state = order::order_status_type::REJECT_AMEND_ORDER_ID_NOT_FOUND;
				_callback(o);
				return;
			}
			auto& ori_odr = *(it->second);
			if (order::oder_engine_type::IMPLIED == ori_odr.engine_type)
			{
				o.order_state = order::order_status_type::REJECT_AMEND_ORDER_ID_NOT_FOUND;
				_callback(o);
				return;
			}
			ori_odr.order_action = o.order_action;
			if (get_flag_value(o) == get_flag_value(ori_odr) && o.quantity <= ori_odr.remain_quantity)
			{
				//just change quantity;
				ori_odr.client_order_id = o.client_order_id;
				ori_odr.quantity = o.quantity;
				ori_odr.display_quantity = o.display_quantity;
				ori_odr.remain_quantity = ori_odr.quantity;
				_callback(ori_odr);
			}
			else
			{
				handle_cancel(o);
				handle_new(o);
			}
		}
	};
}




#endif /* MATCHING_INC_ENGINE_HPP_ */
