#ifndef MATCHING_INC_ENGINE_HPP_
#define MATCHING_INC_ENGINE_HPP_

#include <stddef.h>
#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <functional>
#include <algorithm>
#include <matching/order.hpp>
#include <core/spin_mutex.hpp>

namespace matching
{
	class engine
	{
	public:
		//tiker, leg1, leg2
		using implied_fun = std::function<bool(order::implied_matche_record&, order::implied_matche_record&, order::implied_matche_record&)>;
	private:
		struct impliter
		{
			engine* leg1_e;
			engine* leg2_e;
			order::order_side leg1_side;
			order::order_side leg2_side;
			implied_fun formula;
			impliter():
				leg1_e(nullptr),
				leg2_e(nullptr),
				leg1_side(order::order_side::BUY),
				leg2_side(order::order_side::BUY),
				formula()
			{
			}
			impliter(const impliter&) = default;
			impliter(impliter&&) = default;
			impliter& operator= (const impliter&) = default;
			impliter& operator= (impliter&&) = default;
			~impliter() = default;
			operator bool()
			{
				if (formula)
					return true;
				else
					return false;
			}
		};
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
		using order_set = std::unordered_set<order*>;
		using bid_book_type = std::map<long long, id_order_map, std::greater<long long>>;
		using ask_book_type = std::map<long long, id_order_map, std::less<long long>>;
		using bid_stop_book_type = std::map<long long, order_set, std::less<long long>>;
		using ask_stop_book_type = std::map<long long, order_set, std::greater<long long>>;
		using callback_type = std::function<void(const order&)>;
		using mutex_set = std::set<core::spin_mutex*>;
	private:
		mutable core::spin_mutex _mutex;
		mutable mutex_set _mutex_set;
		mutable impliter _bid_implier;
		mutable impliter _ask_implier;
		search_order_map _odr_map;
		bid_book_type _bid_book;
		ask_book_type _ask_book;
		bid_stop_book_type _bid_stop_book;
		ask_stop_book_type _ask_stop_book;
		callback_type _callback;
		long long _mini_tick;
	private:
		inline order::implied_matche_order_top_result get_bid_top_quantity(unsigned long long quantity)
		{
			order::implied_matche_order_top_result rt;
			bool stop = false;
			for (auto it1 = _bid_book.begin(); it1 != _bid_book.end(); ++it1)
			{
				auto& m2 = it1->second;
				for (auto it2 = m2.begin(); it2 != m2.end(); ++it2)
				{
					rt.records.push_back(order::implied_matche_record(it2->second));
					rt.total_quantity += it2->second->remain_quantity;
					if (rt.total_quantity >= quantity)
					{
						stop = true;
						break;
					}
				}
				if (stop)
					break;
			}
			return rt;
		}

		inline order::implied_matche_order_top_result get_ask_top_quantity(unsigned long long quantity)
		{
			order::implied_matche_order_top_result rt;
			bool stop = false;
			for (auto it1 = _ask_book.begin(); it1 != _ask_book.end(); ++it1)
			{
				auto& m2 = it1->second;
				for (auto it2 = m2.begin(); it2 != m2.end(); ++it2)
				{
					rt.records.push_back(order::implied_matche_record(it2->second));
					rt.total_quantity += it2->second->remain_quantity;
					if (rt.total_quantity >= quantity)
					{
						stop = true;
						break;
					}
				}
				if (stop)
					break;
			}
			return rt;
		}

		static inline order::implied_matche_order_top_result get_top_quantity(engine* e,
				order::order_side side,
				unsigned long long quantity)
		{
			if (order::order_side::BUY == side)
			{
				return e->get_ask_top_quantity(quantity);
			}
			else
			{
				return e->get_bid_top_quantity(quantity);
			}
		}
		void implied_match(impliter& imp, order::implied_matche_record& self, std::function<void()> matched_before)
		{
			if (!imp)
				return;
			order& o = *self.odr;
			auto leg1_orders = engine::get_top_quantity(imp.leg1_e, imp.leg1_side, self.remain_quantity);
			auto leg2_orders = engine::get_top_quantity(imp.leg2_e, imp.leg2_side, self.remain_quantity);
			if (order::order_time_condition::FOK == o.time_condition)
			{
				if (leg1_orders.total_quantity < self.remain_quantity)
					return;
				if (leg2_orders.total_quantity < self.remain_quantity)
					return;
			}
			std::size_t i = 0;
			std::size_t j = 0;
			while (self.remain_quantity != 0 && i < leg1_orders.records.size() && j < leg2_orders.records.size())
			{
				auto& leg1_odr = leg1_orders.records[i];
				auto& leg2_odr = leg2_orders.records[j];
				if (!imp.formula(self, leg1_odr, leg2_odr))
					break;
				if (0 == leg1_odr.remain_quantity)
					++i;
				if (0 == leg2_odr.remain_quantity)
					++j;
			}
			if (order::order_time_condition::FOK == o.time_condition)
			{
				if (0 != self.remain_quantity)
					return;
			}

			if (matched_before)
				matched_before();
			auto matched_id = get_id();
			auto before_best_bid = imp.leg1_e->get_best_price(imp.leg1_e->_bid_book);
			auto before_best_ask = imp.leg1_e->get_best_price(imp.leg1_e->_ask_book);
			for (auto& odr_matched_record : leg1_orders.records)
			{
				auto& leg_o = *(odr_matched_record.odr);
				leg_o.matched_id = matched_id;
				leg_o.matched_type = order::order_matched_type::MAKER;
				for (auto& matched : odr_matched_record.records)
				{
					leg_o.last_match_price = matched.last_match_price;
					leg_o.last_match_quantity = matched.last_match_quantity;
					leg_o.last_matched_order_id = matched.matched_order_id1;
					leg_o.last_matched_order_id2 = matched.matched_order_id2;
					leg_o.remain_quantity -= matched.last_match_quantity;
					if (0 == leg_o.remain_quantity)
					{
						leg_o.order_state = order::order_status_type::FILLED;
					}
					else
					{
						leg_o.order_state = order::order_status_type::PARTIAL_FILL;
					}
					leg_o.update_display();
					imp.leg1_e->_callback(leg_o);
				}
				leg_o.matched_id = 0;
				if (0 == leg_o.remain_quantity)
				{
					if (order::order_side::BUY == imp.leg1_side)
					{
						imp.leg1_e->full_erase_from_normal_book(imp.leg1_e->_ask_book, leg_o);
					}
					else
					{
						imp.leg1_e->full_erase_from_normal_book(imp.leg1_e->_bid_book, leg_o);
					}
				}
			}
			auto after_best_bid = imp.leg1_e->get_best_price(imp.leg1_e->_bid_book);
			auto after_best_ask = imp.leg1_e->get_best_price(imp.leg1_e->_ask_book);
			imp.leg1_e->handle_stop(before_best_bid,
					before_best_ask,
					after_best_bid,
					after_best_ask);
			before_best_bid = imp.leg2_e->get_best_price(imp.leg2_e->_bid_book);
			before_best_ask = imp.leg2_e->get_best_price(imp.leg2_e->_ask_book);
			for (auto& odr_matched_record : leg2_orders.records)
			{
				auto& leg_o = *(odr_matched_record.odr);
				leg_o.matched_id = matched_id;
				leg_o.matched_type = order::order_matched_type::MAKER;
				for (auto& matched : odr_matched_record.records)
				{
					leg_o.last_match_price = matched.last_match_price;
					leg_o.last_match_quantity = matched.last_match_quantity;
					leg_o.last_matched_order_id = matched.matched_order_id1;
					leg_o.last_matched_order_id2 = matched.matched_order_id2;
					leg_o.remain_quantity -= matched.last_match_quantity;
					if (0 == leg_o.remain_quantity)
					{
						leg_o.order_state = order::order_status_type::FILLED;
					}
					else
					{
						leg_o.order_state = order::order_status_type::PARTIAL_FILL;
					}
					leg_o.update_display();
					imp.leg2_e->_callback(leg_o);
				}
				leg_o.matched_id = 0;
				if (0 == leg_o.remain_quantity)
				{
					if (order::order_side::BUY == imp.leg2_side)
					{
						imp.leg2_e->full_erase_from_normal_book(imp.leg2_e->_ask_book, leg_o);
					}
					else
					{
						imp.leg2_e->full_erase_from_normal_book(imp.leg2_e->_bid_book, leg_o);
					}
				}
			}
			after_best_bid = imp.leg2_e->get_best_price(imp.leg2_e->_bid_book);
			after_best_ask = imp.leg2_e->get_best_price(imp.leg2_e->_ask_book);
			imp.leg2_e->handle_stop(before_best_bid,
						before_best_ask,
						after_best_bid,
						after_best_ask);

			o.matched_id = matched_id;
			o.matched_type = order::order_matched_type::TAKER;
			for (auto& matched : self.records)
			{
				o.last_match_price = matched.last_match_price;
				o.last_match_quantity = matched.last_match_quantity;
				o.last_matched_order_id = matched.matched_order_id1;
				o.last_matched_order_id2 = matched.matched_order_id2;
				o.remain_quantity -= matched.last_match_quantity;
				if (0 == o.remain_quantity)
				{
					o.order_state = order::order_status_type::FILLED;
				}
				else
				{
					o.order_state = order::order_status_type::PARTIAL_FILL;
				}
				o.update_display();
				_callback(o);
			}
			o.matched_id = 0;
		}

	private:
		inline void lock()
		{
			for (auto it = _mutex_set.begin(); it != _mutex_set.end(); ++it)
				(*it)->lock();
		}
		inline void unlock()
		{
			for (auto it = _mutex_set.begin(); it != _mutex_set.end(); ++it)
				(*it)->unlock();
		}
	public:
		engine(callback_type&& callback, long long mini_tick = 1):
			_mutex(),
			_mutex_set(),
			_bid_implier(),
			_ask_implier(),
			_odr_map(),
			_bid_book(),
			_ask_book(),
			_bid_stop_book(),
			_ask_stop_book(),
			_callback(std::move(callback)),
			_mini_tick(mini_tick)
		{
			_mutex_set.insert(&_mutex);
		}
		engine(const engine& e):
			_mutex(),
			_mutex_set(),
			_bid_implier(),
			_ask_implier(),
			_odr_map(e._odr_map),
			_bid_book(e._bid_book),
			_ask_book(e._ask_book),
			_bid_stop_book(e._bid_stop_book),
			_ask_stop_book(e._ask_stop_book),
			_callback(e._callback),
			_mini_tick(e._mini_tick)
		{
			_mutex_set.insert(&_mutex);
		}
		engine(engine&& e):
			_mutex(),
			_mutex_set(),
			_bid_implier(),
			_ask_implier(),
			_odr_map(std::move(e._odr_map)),
			_bid_book(std::move(e._bid_book)),
			_ask_book(std::move(e._ask_book)),
			_bid_stop_book(std::move(e._bid_stop_book)),
			_ask_stop_book(std::move(e._ask_stop_book)),
			_callback(std::move(e._callback)),
			_mini_tick(e._mini_tick)
		{
			_mutex_set.insert(&_mutex);
		}
		engine& operator= (const engine& e)
		{
			_odr_map = e._odr_map;
			_bid_book = e._bid_book;
			_ask_book = e._ask_book;
			_bid_stop_book = e._bid_stop_book;
			_ask_stop_book = e._ask_stop_book;
			_callback = e._callback;
			_mini_tick = e._mini_tick;
			return * this;
		}
		engine& operator= (engine&& e)
		{
			_odr_map = std::move(e._odr_map);
			_bid_book = std::move(e._bid_book);
			_ask_book = std::move(e._ask_book);
			_bid_stop_book = std::move(e._bid_stop_book);
			_ask_stop_book = std::move(e._ask_stop_book);
			_callback = std::move(e._callback);
			_mini_tick = e._mini_tick;
			return * this;
		}
		~engine() = default;
		void handle(order& o)
		{
			lock();
			if (order::order_action_type::NEW == o.order_action)
			{
				o.order_state = order::order_status_type::OPEN;
				auto before_best_bid = get_best_price(_bid_book);
				auto before_best_ask = get_best_price(_ask_book);
				if (handle_new(o))
				{
					auto after_best_bid = get_best_price(_bid_book);
					auto after_best_ask = get_best_price(_ask_book);
					handle_stop(before_best_bid,
							before_best_ask,
							after_best_bid,
							after_best_ask);
				}
			}
			else if (order::order_action_type::CANCEL == o.order_action)
			{
				auto before_best_bid = get_best_price(_bid_book);
				auto before_best_ask = get_best_price(_ask_book);
				if (handle_cancel(o))
				{
					auto after_best_bid = get_best_price(_bid_book);
					auto after_best_ask = get_best_price(_ask_book);
					handle_stop(before_best_bid,
							before_best_ask,
							after_best_bid,
							after_best_ask);
				}
			}
			else
			{
				auto before_best_bid = get_best_price(_bid_book);
				auto before_best_ask = get_best_price(_ask_book);
				if (handle_amend(o))
				{
					auto after_best_bid = get_best_price(_bid_book);
					auto after_best_ask = get_best_price(_ask_book);
					handle_stop(before_best_bid,
							before_best_ask,
							after_best_bid,
							after_best_ask);
				}
			}
			unlock();
		}
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
			ori_it_price->second.erase(odr.order_id);
			if (ori_it_price->second.empty())
			{
				book.erase(ori_it_price);
			}
		}

		template <typename Dummy, typename BookType>
		struct get_price_helper
		{
			static long long get_price(order& odr)
			{
				return odr.buy_stop_trigger_price;
			}
		};

		template <typename Dummy>
		struct get_price_helper<Dummy, ask_stop_book_type>
		{
			static long long get_price(order& odr)
			{
				return odr.sell_stop_trigger_price;
			}
		};


		template <typename Dummy, typename BookType>
		struct cross_book_function_helper
		{
			static long long get_non_cross_price(long long price, long long mini_tick)
			{
				return price + mini_tick;
			}
			static void implied_match(engine* e, order::implied_matche_record& o, std::function<void()> matched_before)
			{
				e->implied_match(e->_bid_implier, o, std::move(matched_before));
			}
		};

		template <typename Dummy>
		struct cross_book_function_helper<Dummy, ask_book_type>
		{
			static long long get_non_cross_price(long long price, long long mini_tick)
			{
				return price - mini_tick;
			}
			static void implied_match(engine* e, order::implied_matche_record& o, std::function<void()> matched_before)
			{
				e->implied_match(e->_ask_implier, o, std::move(matched_before));
			}
		};

		template <typename BookType>
		struct cross_book_function
		{
			static long long get_non_cross_price(long long price, long long mini_tick)
			{
				return cross_book_function_helper<bool, BookType>::get_non_cross_price(price, mini_tick);
			}
			static void implied_match(engine* e, order::implied_matche_record& o, std::function<void()> matched_before = std::function<void()>())
			{
				cross_book_function_helper<bool, BookType>::implied_match(e, o, std::move(matched_before));
			}
		};

		template <typename BookType>
		struct get_stop_price_helper
		{
			static long long get_price(order& odr)
			{
				return get_price_helper<int, BookType>::get_price(odr);
			}
		};

		template <typename BookType>
		static void erase_from_stop_book(BookType& book, order& odr)
		{
			auto ori_it_price = book.find(get_stop_price_helper<BookType>::get_price(odr));
			ori_it_price->second.erase(&odr);
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

		void handle_matched_order(order& o, order& o2)
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
			o.update_display();
			o2.update_display();
			_callback(o2);
			_callback(o);
			o.matched_id = 0;
			o2.matched_id = 0;
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
					auto& m2 = it->second;
					for (auto it2 = m2.begin(); it2 != m2.end(); ++it2)
					{
						auto& o2 = *it2->second;
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
				else
					break;
				if (stop)
					break;
			}
			if (total_matched_quantity != o.remain_quantity)
			{

				order::implied_matche_record self(&o);
				self.remain_quantity -= total_matched_quantity;
				cross_book_function<Book>::implied_match(this, self, [&]()
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
				});
				if (0 != o.remain_quantity)
				{
					o.order_state = order::order_status_type::CANCELED_BY_FOK;
					_callback(o);
				}
				else
					return;
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
					else if (order::order_time_condition::MAKER_ONLY_REPRICE == o.time_condition)
					{
						o.price = cross_book_function<Book>::get_non_cross_price(it->first, _mini_tick);
						return true;
					}
					auto& m2 = it->second;
					for (auto it2 = m2.begin(); it2 != m2.end();)
					{
						auto& o2 = *it2->second;
						handle_matched_order(o, o2);
						if (0 != o2.remain_quantity)
						{
							++it2;
						}
						else
						{
							it2 = m2.erase(it2);
							_odr_map.erase(o2.order_id);
						}
						if (0 == o.remain_quantity)
						{
							stop = true;
							break;
						}
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
			if (0 != o.remain_quantity)
			{
				order::implied_matche_record self(&o);
				cross_book_function<Book>::implied_match(this, self);
			}
			if (0 != o.remain_quantity)
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
				self[o.price][o.order_id] = &((_odr_map.emplace(o.order_id, o).first)->second);
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

		inline void init_new_order(order& o)
		{
			o.order_id = get_id();
			o.remain_quantity = o.quantity;
		}

		inline bool handle_new(order& o)
		{
			o.order_id = 0;
			if (0 == o.quantity)
			{
				o.order_state = order::order_status_type::REJECT_QUANTITY_ZERO;
				_callback(o);
				return false;
			}
			if (o.display_quantity > o.quantity)
			{
				o.order_state = order::order_status_type::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY;
				_callback(o);
				return false;
			}
			if (order::order_side::BUY == o.side)
			{
				init_new_order(o);
				handle_normal_new(_bid_book, _ask_book, o);
				return true;
			}
			else if (order::order_side::SELL == o.side)
			{
				init_new_order(o);
				handle_normal_new(_ask_book, _bid_book, o);
				return true;
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
					init_new_order(o);
					_bid_stop_book[o.buy_stop_trigger_price].insert(&((_odr_map.emplace(o.order_id, o).first)->second));
					_callback(o);
				}
				return false;
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
					init_new_order(o);
					_ask_stop_book[o.sell_stop_trigger_price].insert(&((_odr_map.emplace(o.order_id, o).first)->second));
					_callback(o);
				}
				return false;
			}
			else if (order::order_side::BUY_SELL_STOP == o.side)
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
					init_new_order(o);
					auto podr = &((_odr_map.emplace(o.order_id, o).first)->second);
					_bid_stop_book[o.buy_stop_trigger_price].insert(podr);
					_ask_stop_book[o.sell_stop_trigger_price].insert(podr);
					_callback(o);
				}
				return false;
			}
			else
			{
				o.order_state = order::order_status_type::REJECT_UNKNOW_ORDER_ACTION;
				_callback(o);
				return false;
			}
		}

		inline bool handle_cancel(order& o)
		{
			auto it = _odr_map.find(o.order_id);
			if (_odr_map.end() == it)
			{
				o.order_state = order::order_status_type::REJECT_CANCEL_ORDER_ID_NOT_FOUND;
				_callback(o);
				return false;
			}
			auto& ori_odr = it->second;
			ori_odr.client_order_id = o.client_order_id;
			ori_odr.order_state = order::order_status_type::CANCELED_BY_USER;
			_callback(ori_odr);
			bool is_impact_order = true;
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
				is_impact_order = false;
			}
			else if (order::order_side::SELL_STOP == ori_odr.side)
			{
				erase_from_stop_book(_ask_stop_book, ori_odr);
				is_impact_order = false;
			}
			else
			{
				erase_from_stop_book(_bid_stop_book, ori_odr);
				erase_from_stop_book(_ask_stop_book, ori_odr);
				is_impact_order = false;
			}
			_odr_map.erase(it);
			return is_impact_order;
		}

		//can keep the priority if just reduce quantity
		inline bool handle_amend(order& o)
		{
			if (0 == o.quantity)
			{
				o.order_state = order::order_status_type::REJECT_QUANTITY_ZERO;
				_callback(o);
				return false;
			}
			if (o.display_quantity > o.quantity)
			{
				o.order_state = order::order_status_type::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY;
				_callback(o);
				return false;
			}
			auto it = _odr_map.find(o.order_id);
			if (_odr_map.end() == it)
			{
				o.order_state = order::order_status_type::REJECT_AMEND_ORDER_ID_NOT_FOUND;
				_callback(o);
				return false;
			}
			auto& ori_odr = it->second;
			ori_odr.order_action = o.order_action;
			if (order::can_amend(o, ori_odr))
			{
				ori_odr.client_order_id = o.client_order_id;
				ori_odr.quantity = o.quantity;
				ori_odr.display_quantity = o.display_quantity;
				ori_odr.remain_quantity = ori_odr.quantity;
				_callback(ori_odr);
				//No order impact
				return false;
			}
			else
			{
				handle_cancel(o);
				return handle_new(o);
			}
		}

		template<typename Book>
		inline long long get_best_price(Book& book)
		{
			auto it = book.begin();
			if (book.end() == it)
			{
				return order::MARKET_PRICE;
			}
			else
			{
				return it->first;
			}
		}

		template<typename Cmp>
		inline bool check_stop_trigger(long long before, long long after)
		{
			if (order::MARKET_PRICE == before && order::MARKET_PRICE != after)
			{
				return true;
			}
			else if (order::MARKET_PRICE != before && order::MARKET_PRICE == after)
			{
				return true;
			}
			else if (before == after)
			{
				return false;
			}
			else
			{
				Cmp cmp;
				if (cmp(after, before))
				{
					return false;
				}
				else
				{
					return true;
				}
			}
		}

		template <typename Book, typename CrossBook, std::size_t offSet>
		inline void handle_stop(Book& book, CrossBook& cross_book, long long limited_price)
		{
			typename Book::key_compare cmp;
			std::map<unsigned long long, order*> stop_list;
			do
			{
				stop_list.clear();
				for (auto it = book.begin(); it != book.end(); ++it)
				{
					if (!cmp(limited_price, it->first))
					{
						for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
						{
							stop_list[(*it2)->order_id] = (*it2);
						}
					}
					else
						break;
				}
				for (auto it = stop_list.begin(); it != stop_list.end(); ++it)
				{
					auto& o = *(it->second);
					if (!handle_cross<CrossBook, offSet>(cross_book, o))
					{
						full_erase_from_stop_book(book, o);
					}
				}
				auto it = cross_book.begin();
				if (cross_book.end() == it)
				{
					break;
				}
				else
				{
					limited_price = it->first;
				}
			}
			while(!stop_list.empty());
		}

		inline void handle_stop(long long before_best_bid,
				long long before_best_ask,
				long long after_best_bid,
				long long after_best_ask)
		{
			if (check_stop_trigger<typename decltype(_bid_book)::key_compare>(before_best_bid, after_best_bid))
			{
				handle_stop<
						decltype(_ask_stop_book),
						decltype(_bid_book),
						offsetof(order, sell_stop_limited_price)>
				(_ask_stop_book, _bid_book, after_best_bid);
			}
			else if (check_stop_trigger<typename decltype(_ask_book)::key_compare>(before_best_ask, after_best_ask))
			{
				handle_stop<
						decltype(_bid_stop_book),
						decltype(_ask_book),
						offsetof(order, buy_stop_limited_price)>
				(_bid_stop_book, _ask_book, after_best_ask);
			}
		}
	public:
		inline void set_bid_implier(engine& leg1_e,
				engine& leg2_e,
				order::order_side leg1_side,
				order::order_side leg2_side,
				implied_fun&& formula)
		{
			if (!formula)
			{
				return;
			}
			_mutex_set.insert(&(leg1_e._mutex));
			_mutex_set.insert(&(leg2_e._mutex));
			_bid_implier.leg1_e = &leg1_e;
			_bid_implier.leg2_e = &leg2_e;
			_bid_implier.leg1_side = leg1_side;
			_bid_implier.leg2_side = leg2_side;
			_bid_implier.formula = std::move(formula);
		}
		inline void unset_bid_implier()
		{
			if (!_bid_implier.formula)
			{
				return;
			}
			_bid_implier.formula = implied_fun();
			_mutex_set.erase(&(_bid_implier.leg1_e->_mutex));
			_mutex_set.erase(&(_bid_implier.leg2_e->_mutex));
			_bid_implier.leg1_e = nullptr;
			_bid_implier.leg2_e = nullptr;
		}
		inline void set_ask_implier(engine& leg1_e,
				engine& leg2_e,
				order::order_side leg1_side,
				order::order_side leg2_side,
				implied_fun&& formula)
		{
			if (!formula)
			{
				return;
			}
			_mutex_set.insert(&(leg1_e._mutex));
			_mutex_set.insert(&(leg2_e._mutex));
			_ask_implier.leg1_e = &leg1_e;
			_ask_implier.leg2_e = &leg2_e;
			_ask_implier.leg1_side = leg1_side;
			_ask_implier.leg2_side = leg2_side;
			_ask_implier.formula = std::move(formula);
		}
		inline void unset_ask_implier()
		{
			if (!_ask_implier.formula)
			{
				return;
			}
			_ask_implier.formula = implied_fun();
			_mutex_set.erase(&(_ask_implier.leg1_e->_mutex));
			_mutex_set.erase(&(_ask_implier.leg2_e->_mutex));
			_ask_implier.leg1_e = nullptr;
			_ask_implier.leg2_e = nullptr;
		}
	};
}




#endif /* MATCHING_INC_ENGINE_HPP_ */
