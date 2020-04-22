#ifndef MATCHING_INC_ENGINE_HPP_
#define MATCHING_INC_ENGINE_HPP_

#include <stddef.h>
#include <map>
#include <vector>
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
		using search_order_map = std::unordered_map<unsigned long long, order>;
		using id_order_map = std::map<unsigned long long, order*>;
		using type_order_map = std::map<order::order_engine_type, id_order_map>;
		using bid_book_type = std::map<long long, type_order_map, std::greater<long long>>;
		using ask_book_type = std::map<long long, type_order_map, std::less<long long>>;
		using bid_stop_book_type = std::map<long long, id_order_map, std::greater<long long>>;
		using ask_stop_book_type = std::map<long long, id_order_map, std::less<long long>>;
		using callback_type = std::function<void(const order&)>;
	private:
		search_order_map _odr_map;
		bid_book_type _bid_book;
		ask_book_type _ask_book;
		bid_stop_book_type _bid_stop_book;
		ask_stop_book_type _ask_stop_book;
		callback_type _callback;
	public:
		engine(callback_type&& callback);
		~engine();
		void handle(order& o, order::order_engine_type engine_type = order::order_engine_type::NORMAL);
		inline void recovery(callback_type&& callback) const
		{
			for (const auto& item : _odr_map)
			{
				callback(item.second);
			}
		}
	private:
		template <typename BookType>
		static void erase_from_normal_book(BookType& book, order& odr)
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

		template <typename BookType>
		static void erase_from_stop_book(BookType& book, order& odr)
		{
			auto ori_it_price = book.find(odr.price);
			ori_it_price->second.erase(odr.order_id);
			if (ori_it_price->second.empty())
			{
				book.erase(ori_it_price);
			}
		}

		template <typename BookType>
		void full_erase_from_normal_book(BookType& book, order& odr)
		{
			erase_from_normal_book(book, odr);
			_odr_map.erase(odr.order_id);
		}

		template <typename BookType>
		void full_erase_from_stop_book(BookType& book, order& odr)
		{
			erase_from_stop_book(book, odr);
			_odr_map.erase(odr.order_id);
		}

		inline void handle_matched_implied(order&)
		{
		}

		inline void handle_matched_order(order& o, order& o2)
		{
			auto matched_price = o2.price;
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
			handle_matched_implied(o2);
		}

		template <typename Book, std::size_t offSet>
		inline void handle_fok_cross(Book& book, order& o)
		{
			const long long& limited_price = *static_cast<long long*>(static_cast<void*>(static_cast<char*>(static_cast<void*>(&o)) + offSet));
			std::vector<order*> cross_odrs;
			unsigned long long total_matched_quantity = 0;
			typename Book::key_compare cross_cmp;
			bool stop = false;
			for (auto it = book.begin(); it != book.end(); ++it)
			{
				if (!cross_cmp(limited_price, it->first) || order::MARKET_PRICE == limited_price)
				{
					auto m2 = it->second;
					for (auto it2 = m2.begin(); it2 != m2.end(); ++it2)
					{
						auto m3 = it2->second;
						for (auto it3 = m3.begin(); it3 != m3.end(); ++it3)
						{
							auto& o2 = *it3->second;
							auto matched_quantity = std::min(o.remain_quantity, o2.remain_quantity);
							total_matched_quantity += matched_quantity;
							cross_odrs.push_back(&o2);
							if (total_matched_quantity == o.remain_quantity)
							{
								stop = true;
								break;
							}
						}
						if (stop)
							break;
					}
				}
				else
					break;
				if (stop)
					break;
			}
			if (total_matched_quantity != o.remain_quantity)
			{
				o.order_state = order::order_status_type::CANCELED_BY_FOK;
				_callback(o);
			}
			else
			{
				for (std::size_t i = 0; i < cross_odrs.size(); ++i)
				{
					auto& o2 = *cross_odrs[i];
					handle_matched_order(o, o2);
					if (0 == o2.remain_quantity)
					{
						full_erase_from_normal_book(book, o2);
					}
				}
			}
		}

		template <typename Book, std::size_t offSet>
		inline bool handle_cross(Book& book, order& o)
		{
			const long long& limited_price = *static_cast<long long*>(static_cast<void*>(static_cast<char*>(static_cast<void*>(&o)) + offSet));
			o.remain_quantity = o.quantity;
			if (order::order_time_condition::FOK == o.time_condition)
			{
				handle_fok_cross<Book, offSet>(book, o);
				return false;
			}
			typename Book::key_compare cross_cmp;
			if (order::MARKET_PRICE == limited_price && book.empty())
			{
				o.order_state = order::order_status_type::CANCELED_BY_MARKET_ORDER_NOTHING_MATCH;
				_callback(o);
				return false;
			}
			bool stop = false;
			for (auto it = book.begin(); it != book.end();)
			{
				if (!cross_cmp(limited_price, it->first) || order::MARKET_PRICE == limited_price)
				{
					if (order::order_time_condition::MAKER_ONLY == o.time_condition)
					{
						o.order_state = order::order_status_type::CANCELED_BY_MAKER_ONLY;
						_callback(o);
						return false;
					}
					auto m2 = it->second;
					for (auto it2 = m2.begin(); it2 != m2.end();)
					{
						auto m3 = it2->second;
						for (auto it3 = m3.begin(); it3 != m3.end();)
						{
							auto& o2 = *it3->second;
							handle_matched_order(o, o2);
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
						it = book.erase(it);
					}
				}
				else
					break;
				if (stop)
					break;
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
					return false;
				}
				if (order::MARKET_PRICE == limited_price && 0 != o.remain_quantity)
				{
					o.order_state = order::order_status_type::CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED;
					_callback(o);
					return false;
				}
				else if (o.remain_quantity == o.quantity)
					_callback(o);
				return true;
			}
			return false;
		}

		template <typename SelfBook,typename CrossBook>
		inline void handle_normal_new(SelfBook& self, CrossBook& cross, order& o)
		{
			if (handle_cross<CrossBook, offsetof(order, price)>(cross, o))
			{
				self[o.price][o.engine_type][o.order_id] = &((_odr_map.emplace(o.order_id, o).first)->second);
			}
		}

		template<typename Book,
			std::size_t TriggeroffSet,
			std::size_t LimitoffSet,
			order::order_status_type RejectStatus1,
			order::order_status_type RejectStatus2,
			order::order_status_type RejectStatus3>
		bool stop_check(Book& book, order& o)
		{
			const long long& trigger_price = *static_cast<long long*>(static_cast<void*>(static_cast<char*>(static_cast<void*>(&o)) + TriggeroffSet));
			const long long& limited_price = *static_cast<long long*>(static_cast<void*>(static_cast<char*>(static_cast<void*>(&o)) + LimitoffSet));
			typename Book::key_compare cross_cmp;
			if (limited_price != order::MARKET_PRICE)
			{
				if (cross_cmp(limited_price, trigger_price))
				{
					o.order_state = RejectStatus1;
					_callback(o);
					return false;
				}
			}
			auto it = book.begin();
			if (book.end() == it)
			{
				o.order_state = RejectStatus2;
				_callback(o);
				return false;
			}
			if (!cross_cmp(it->first, trigger_price))
			{
				o.order_state = RejectStatus3;
				_callback(o);
				return false;
			}
			return true;
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
				handle_normal_new(_bid_book, _ask_book, o);
			}
			else if (order::order_side::SELL == o.side)
			{
				handle_normal_new(_ask_book, _bid_book, o);
			}
			else if (order::order_side::BUY_STOP == o.side)
			{
				if (stop_check<decltype(_ask_book),
						offsetof(order, buy_stop_trigger_price),
						offsetof(order, buy_stop_limited_price),
						order::order_status_type::REJECT_BUY_STOP_TRIGGER_LESS_THAN_STOP_LIMITED,
						order::order_status_type::REJECT_BUY_STOP_NO_BEST_ASK,
						order::order_status_type::REJECT_BUY_STOP_TRIGGER_LESS_THAN_BEST_ASK>(_ask_book,o))
				{
					_bid_stop_book[o.buy_stop_trigger_price][o.order_id] = &((_odr_map.emplace(o.order_id, o).first)->second);
				}
			}
			else if (order::order_side::SELL_STOP == o.side)
			{
				if (stop_check<decltype(_bid_book),
						offsetof(order, sell_stop_trigger_price),
						offsetof(order, sell_stop_limited_price),
						order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED,
						order::order_status_type::REJECT_SELL_STOP_NO_BEST_BID,
						order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_BEST_BID>(_bid_book,o))
				{
					_ask_stop_book[o.sell_stop_trigger_price][o.order_id] = &((_odr_map.emplace(o.order_id, o).first)->second);
				}
			}
			else
			{
				if (o.buy_stop_trigger_price <= o.sell_stop_trigger_price)
				{
					o.order_state = order::order_status_type::REJECT_BUY_SELL_STOP_TRIGGER_CROSS;
					_callback(o);
				}
				else if(stop_check<decltype(_ask_book),
						offsetof(order, buy_stop_trigger_price),
						offsetof(order, buy_stop_limited_price),
						order::order_status_type::REJECT_BUY_STOP_TRIGGER_LESS_THAN_STOP_LIMITED,
						order::order_status_type::REJECT_BUY_STOP_NO_BEST_ASK,
						order::order_status_type::REJECT_BUY_STOP_TRIGGER_LESS_THAN_BEST_ASK>(_ask_book,o) &&
						stop_check<decltype(_bid_book),
						offsetof(order, sell_stop_trigger_price),
						offsetof(order, sell_stop_limited_price),
						order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED,
						order::order_status_type::REJECT_SELL_STOP_NO_BEST_BID,
						order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_BEST_BID>(_bid_book,o))
				{
					auto podr = &((_odr_map.emplace(o.order_id, o).first)->second);
					_bid_stop_book[o.buy_stop_trigger_price][o.order_id] = podr;
					_ask_stop_book[o.sell_stop_trigger_price][o.order_id] = podr;
				}
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
			auto& ori_odr = it->second;
			if (order::order_engine_type::IMPLIED == ori_odr.engine_type)
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
				erase_from_normal_book(_bid_book, ori_odr);
			}
			else if (order::order_side::SELL == ori_odr.side)
			{
				erase_from_normal_book(_ask_book, ori_odr);
			}
			else if (order::order_side::BUY_STOP == ori_odr.side)
			{
				erase_from_stop_book(_bid_stop_book, ori_odr);
			}
			else if (order::order_side::SELL_STOP == ori_odr.side)
			{
				erase_from_stop_book(_ask_stop_book, ori_odr);
			}
			else
			{
				erase_from_stop_book(_bid_stop_book, ori_odr);
				erase_from_stop_book(_ask_stop_book, ori_odr);
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
			auto& ori_odr = it->second;
			if (order::order_engine_type::IMPLIED == ori_odr.engine_type)
			{
				o.order_state = order::order_status_type::REJECT_AMEND_ORDER_ID_NOT_FOUND;
				_callback(o);
				return;
			}
			ori_odr.order_action = o.order_action;
			if (o.get_flag_value() == ori_odr.get_flag_value() && o.quantity <= ori_odr.remain_quantity)
			{
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
