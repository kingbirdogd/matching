#ifndef MATCHING_INC_ENGINE_HPP_
#define MATCHING_INC_ENGINE_HPP_

#include <map>
#include <unordered_map>
#include <atomic>
#include <functional>
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
	private:
		search_order_map _odr_map;
		bid_book_type _bid_book;
		ask_book_type _ask_book;
		callback_type _callback;
	public:
		engine(callback_type&& callback);
		~engine();
		void handle(order& o);
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
			return (*static_cast<const unsigned long long*>(static_cast<const void*>(&odr.side)))
					& 0x00FFFFFFFFFFFFFF; //ignore last byte, order_state
		}

		template <typename BookSelf, typename BookCross>
		inline void handle_new(BookSelf&, BookCross&, order& o)
		{
			if (order::order_stop_type::CUT_LOST_TAKE_PROFIT_WITHOUT_POSITION == o.stop_type)
			{
				if (order::order_stop_condition_type::ABSOLUTE == o.cut_lost_condition)
				{
				}
				else
				{
				}

				if (order::order_stop_condition_type::ABSOLUTE == o.take_profit_condition)
				{
				}
				else
				{
				}

				/*
				if (cross_book.empty())
				{
					o.order_state = order::order_status_type::REJECT_STOP_VALUE_HAS_NO_BEST_PRICE;
					_callback(o);
					return;
				}
				*/
			}
		}

		inline void handle_new(order& o)
		{
			if (o.display_quantity > o.quantity)
			{
				o.order_state = order::order_status_type::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY;
				_callback(o);
				return;
			}
			if (order::order_side::BUY == o.side)
			{
				handle_new(_bid_book, _ask_book, o);
			}
			else
			{
				handle_new(_ask_book, _bid_book, o);
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
