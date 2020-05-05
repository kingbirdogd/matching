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
		class matcher;
	private:
		class iterator
		{
		private:
			friend matcher;
		private:
			using price_it = typename bid_book_type::iterator;
			using price_odr = typename id_order_map::iterator;
		private:
			engine* _e;
			order::order_side _size;
			price_it _it1;
			price_odr _it2;
			unsigned long long _remain_quantity;
		public:
			iterator():
				_e(nullptr),
				_size(order::order_side::BUY),
				_it1(),
				_it2(),
				_remain_quantity(0)
			{
			}
			iterator(engine* e, order::order_side side):
				_e(e),
				_size(side),
				_it1(),
				_it2()
			{
				reset();
			}
			iterator(const iterator&) = default;
			iterator(iterator&&) = default;
			iterator& operator= (const iterator&) = default;
			iterator& operator= (iterator&&) = default;
			~iterator() = default;
			void reset()
			{
				if(order::order_side::BUY == _size)
				{
					_it1 = _e->_ask_book.begin();
					if (_e->_ask_book.end() != _it1)
					{
						_it2 = _it1->second.begin();
						_remain_quantity = _it2->second->remain_quantity;
					}
				}
				else
				{
					_it1 = _e->_bid_book.begin();
					if (_e->_bid_book.end() != _it1)
					{
						_it2 = _it1->second.begin();
						_remain_quantity = _it2->second->remain_quantity;
					}
				}
			}
			void next()
			{
				++_it2;
				if (_it1->second.end() == _it2)
				{
					++_it1;
					if(order::order_side::BUY == _size)
					{
						if (_e->_ask_book.end() != _it1)
						{
							_it2 = _it1->second.begin();
							_remain_quantity = _it2->second->remain_quantity;
						}
					}
					else
					{
						if (_e->_bid_book.end() != _it1)
						{
							_it2 = _it1->second.begin();
							_remain_quantity = _it2->second->remain_quantity;
						}
					}
				}
				else
				{
					_remain_quantity = _it2->second->remain_quantity;
				}
			}
			void previous()
			{
				if (_it1->second.begin() == _it2)
				{
					if(order::order_side::BUY == _size)
					{
						if (_e->_ask_book.begin() != _it1)
						{
							--_it1;
							auto r_it = _it1->second.rbegin();
							_it2 = _it1->second.find(r_it->second->order_id);
							_remain_quantity = _it2->second->remain_quantity;
						}
					}
					else
					{
						if (_e->_bid_book.begin() != _it1)
						{
							--_it1;
							auto r_it = _it1->second.rbegin();
							_it2 = _it1->second.find(r_it->second->order_id);
							_remain_quantity = _it2->second->remain_quantity;
						}
					}
				}
				else
				{
					--_it2;
					_remain_quantity = _it2->second->remain_quantity;
				}
			}
			iterator& operator++()
			{
				next();
				return *this;
			}
			iterator operator++(int)
			{
				iterator rt(*this);
				next();
				return rt;
			}
			iterator& operator--()
			{
				previous();
				return *this;
			}
			iterator operator--(int)
			{
				iterator rt(*this);
				previous();
				return rt;
			}
			iterator& operator+=(std::size_t size)
			{
				for (std::size_t i = 0; i < size; ++i)
					next();
				return *this;
			}
			iterator& operator-=(std::size_t size)
			{
				for (std::size_t i = 0; i < size; ++i)
					previous();
				return *this;
			}
			iterator operator+(std::size_t size) const
			{
				iterator rt(*this);
				for (std::size_t i = 0; i < size; ++i)
					rt.next();
				return rt;
			}
			iterator operator-(std::size_t size) const
			{
				iterator rt(*this);
				for (std::size_t i = 0; i < size; ++i)
					rt.previous();
				return rt;
			}
			iterator& operator[](std::size_t idx)
			{
				reset();
				for (std::size_t i = 0; i < idx; ++i)
					next();
				return *this;
			}
			std::size_t price_size()
			{
				if(order::order_side::BUY == _size)
				{
					return _e->_ask_book.size();
				}
				else
				{
					return _e->_bid_book.size();
				}
			}
			std::size_t order_size()
			{
				std::size_t sum = 0;
				if(order::order_side::BUY == _size)
				{
					for (auto it = _e->_ask_book.begin(); it != _e->_ask_book.end(); ++it)
					{
						sum += it->second.size();
					}
					return sum;
				}
				else
				{
					for (auto it = _e->_bid_book.begin(); it != _e->_bid_book.end(); ++it)
					{
						sum += it->second.size();
					}
					return sum;
				}
			}
			std::size_t order_size(long long limited_price)
			{
				std::size_t sum = 0;
				if(order::order_side::BUY == _size)
				{
					for (auto it = _e->_ask_book.begin(); it != _e->_ask_book.end() && it->first <= limited_price; ++it)
					{
						sum += it->second.size();
					}
					return sum;
				}
				else
				{
					for (auto it = _e->_bid_book.begin(); it != _e->_bid_book.end() && it->first >= limited_price; ++it)
					{
						sum += it->second.size();
					}
					return sum;
				}
			}
			std::size_t quantity()
			{
				std::size_t sum = 0;
				if(order::order_side::BUY == _size)
				{
					for (auto it = _e->_ask_book.begin(); it != _e->_ask_book.end(); ++it)
					{
						for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
						{
							sum += it2->second->remain_quantity;
						}
					}
					return sum;
				}
				else
				{
					for (auto it = _e->_bid_book.begin(); it != _e->_bid_book.end(); ++it)
					{
						for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
						{
							sum += it2->second->remain_quantity;
						}
					}
					return sum;
				}
			}
			std::size_t quantity(long long limited_price)
			{
				std::size_t sum = 0;
				if(order::order_side::BUY == _size)
				{
					for (auto it = _e->_ask_book.begin(); it != _e->_ask_book.end() && it->first <= limited_price; ++it)
					{
						for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
						{
							sum += it2->second->remain_quantity;
						}
					}
					return sum;
				}
				else
				{
					for (auto it = _e->_bid_book.begin(); it != _e->_bid_book.end() && it->first >= limited_price; ++it)
					{
						for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
						{
							sum += it2->second->remain_quantity;
						}
					}
					return sum;
				}
			}
			void erase()
			{
				if (!valid())
					return;
				auto odr_id = _it2->second->order_id;
				_it2 = _it1->second.erase(_it2);
				if (_it1->second.empty())
				{
					if(order::order_side::BUY == _size)
					{
						_it1 = _e->_ask_book.erase(_it1);
						if (_e->_ask_book.end() != _it1)
						{
							_it2 = _it1->second.begin();
							_remain_quantity = _it2->second->remain_quantity;
						}
					}
					else
					{
						_it1 = _e->_bid_book.erase(_it1);
						if (_e->_bid_book.end() != _it1)
						{
							_it2 = _it1->second.begin();
							_remain_quantity = _it2->second->remain_quantity;
						}
					}
				}
				else if (_it1->second.end() == _it2)
				{
					++_it1;
					if(order::order_side::BUY == _size)
					{
						if (_e->_ask_book.end() != _it1)
						{
							_it2 = _it1->second.begin();
							_remain_quantity = _it2->second->remain_quantity;
						}
					}
					else
					{
						if (_e->_bid_book.end() != _it1)
						{
							_it2 = _it1->second.begin();
							_remain_quantity = _it2->second->remain_quantity;
						}
					}
				}
				else
				{
					_remain_quantity = _it2->second->remain_quantity;
				}
				_e->_odr_map.erase(odr_id);
			}
			order& get_order()
			{
				return *(_it2->second);
			}
			operator order& ()
			{
				return get_order();
			}
			order* operator->()
			{
				return _it2->second;
			}
			const order* operator->() const
			{
				return _it2->second;
			}
			bool valid() const
			{
				if (!_e)
					return false;
				if(order::order_side::BUY == _size)
				{
					return (_e->_ask_book.end() != _it1);
				}
				else
				{
					return (_e->_bid_book.end() != _it1);
				}
			}
			operator bool()
			{
				return valid();
			}
			core::spin_mutex& get_mutex()
			{
				return _e->_mutex;
			}
			void maker_match(
					unsigned long long matched_id,
					unsigned long long last_match_quantity,
					unsigned long long last_matched_order_id,
					unsigned long long last_matched_order_id2 = 0)
			{
				auto& o = get_order();
				engine::maker_match(o, matched_id,
						last_match_quantity,
						last_matched_order_id,
						last_matched_order_id2);
				_e->callback(o);
				if (0 == o.remain_quantity)
				{
					erase();
				}
			}
			void taker_match(order& o,
					unsigned long long matched_id,
					long long last_match_price,
					unsigned long long last_match_quantity,
					unsigned long long last_matched_order_id,
					unsigned long long last_matched_order_id2 = 0)
			{
				engine::match(o,
						order::order_matched_type::TAKER,
						matched_id,
						last_match_price,
						last_match_quantity,
						last_matched_order_id,
						last_matched_order_id2);
				_e->callback(o);
			}
			long long top_price() const
			{
				if (!valid())
					return order::MARKET_PRICE;
				else
					return _it2->second->price;
			}
			unsigned long long top_quantity() const
			{
				if (!valid())
					return 0;
				else
					return _it2->second->remain_quantity;
			}
			unsigned long long top_matching_quantity() const
			{
				if (!valid())
					return 0;
				else
					return _remain_quantity;
			}

			void reduce_matching(unsigned long long quantity)
			{
				_remain_quantity -= quantity;
			}
		};
	public:
		class implier_base
		{
		private:
			friend matcher;
		private:
			unsigned long long _priority;
			iterator _leg1;
			iterator _leg2;
		public:
			implier_base() = delete;
			implier_base
			(
				unsigned long long priority,
				engine* leg1_e,
				engine* leg2_e,
				order::order_side leg1_side,
				order::order_side leg2_side
			):
				_priority(priority),
				_leg1(iterator(leg1_e, leg1_side)),
				_leg2(iterator(leg2_e, leg2_side))
			{
			}
			implier_base(const implier_base& imp) = default;
			implier_base(implier_base&& imp) = default;
			implier_base& operator= (const implier_base& imp) = default;
			implier_base& operator= (implier_base&& imp)  = default;
			virtual ~implier_base() = default;
			//if nothing match return order::MARKET_PRICE;
			virtual long long matchd_price(long long leg1_price, long long leg2_price, unsigned long long mini_tick) = 0;
		private:
			void reset()
			{
				_leg1.reset();
				_leg2.reset();
			}
		};
	private:
		struct matched_record
		{
			long long matched_price;
			long long matched_quantity;
			iterator leg1;
			iterator leg2;
			matched_record():
				matched_price(order::MARKET_PRICE),
				matched_quantity(0),
				leg1(),
				leg2()
			{
			}
			matched_record(const matched_record&) = default;
			matched_record(matched_record&&) = default;
			matched_record& operator= (const matched_record&) = default;
			matched_record& operator= (matched_record&&) = default;
			~matched_record() = default;
			operator bool()
			{
				return (order::MARKET_PRICE != matched_price);
			}
		};
	private:
		class matcher
		{
		private:
			iterator _local;
			std::map<unsigned long long, implier_base*, std::greater<unsigned long long>> _impliers;
		private:
			matcher(engine* e, order::order_side side):
				_local(e, side),
				_impliers()
			{
				_local._e->_mutex_set.insert(&(_local._e->_mutex));
			}
			matcher() = delete;
			matcher(const matcher&) = delete;
			matcher(matcher&&) = delete;
			matcher& operator= (const matcher&) = delete;
			matcher& operator= (matcher&&) = delete;
			~matcher() = default;
			void unset_implier(implier_base* imp)
			{
				auto it = _impliers.find(imp->_priority);
				if (_impliers.end() != it)
				{
					auto& leg1 = it->second->_leg1;
					auto& leg2 = it->second->_leg2;
					if (leg1)
					{
						_local._e->_mutex_set.erase(&(leg1._e->_mutex));
					}
					if (leg2)
					{
						_local._e->_mutex_set.erase(&(leg2._e->_mutex));
					}
					_impliers.erase(it);
				}
			}
			void set_implier(implier_base* imp)
			{
				unset_implier(imp);
				_impliers.emplace(imp->_priority, imp);
				auto& leg1 = imp->_leg1;
				auto& leg2 = imp->_leg2;
				if (leg1)
				{
					_local._e->_mutex_set.insert(&(leg1._e->_mutex));
				}
				if (leg2)
				{
					_local._e->_mutex_set.insert(&(leg2._e->_mutex));
				}
			}
			void reset()
			{
				_local.reset();
				for (auto it = _impliers.begin(); it != _impliers.end(); ++it)
				{
					it->second->_leg1.reset();
					it->second->_leg2.reset();
				}
			}
			template <typename CMP>
			long long top_price(const CMP& cmp) const
			{
				auto price = _local.top_price();
				for (auto it = _impliers.begin(); it != _impliers.end(); ++it)
				{
					auto matched_price = it->second->matchd_price(it->second->_leg1->price,
							it->second->_leg2->price,
							_local._e->_mini_tick);
					if (order::MARKET_PRICE != matched_price)
					{
						if (order::MARKET_PRICE == price || cmp(matched_price, price))
						{
							price = matched_price;
						}
					}
				}
				return price;
			}
			template <typename CMP>
			auto top(const CMP& cmp, long long& top_price, unsigned long long& top_quantity) const
			{
				auto rt = _impliers.end();
				top_price = _local.top_price();
				top_quantity = _local.top_matching_quantity();
				for (auto it = _impliers.begin(); it != _impliers.end(); ++it)
				{
					auto matched_price = it->second->matchd_price(it->second->_leg1->price,
							it->second->_leg2->price,
							_local._e->_mini_tick);
					if (order::MARKET_PRICE != matched_price)
					{
						if (order::MARKET_PRICE == top_price || cmp(matched_price, top_price))
						{
							top_price = matched_price;
							top_quantity = it->second->_leg1.top_matching_quantity() < it->second->_leg2.top_matching_quantity()
									? it->second->_leg1.top_matching_quantity() : it->second->_leg2.top_matching_quantity();
							rt = it;
						}
					}
				}
				return rt;
			}
			template <typename CMP>
			matched_record match(const CMP& cmp, long long limited_price, unsigned long long quantity)
			{
				matched_record rt;
				auto it = top(cmp, rt.matched_price, rt.matched_quantity);
				if (rt)
				{
					if ((order::MARKET_PRICE != limited_price) && cmp(limited_price, rt.matched_price))
					{
						rt.matched_price = order::MARKET_PRICE;
						return rt;
					}
					if (rt.matched_quantity < quantity)
						rt.matched_quantity = quantity;
					if (_impliers.end() == it)
					{
						rt.leg1 = _local;
						if (rt.matched_quantity == _local.top_matching_quantity())
						{
							++_local;
						}
					}
					else
					{
						auto& imp = (*it->second);
						rt.leg1 = imp._leg1;
						rt.leg2 = imp._leg2;
						imp._leg1.reduce_matching(rt.matched_quantity);
						imp._leg2.reduce_matching(rt.matched_quantity);
						if (0 == imp._leg1.top_matching_quantity())
						{
							++(imp._leg1);
						}
						if (0 == imp._leg2.top_matching_quantity())
						{
							++(imp._leg2);
						}
					}
				}
				return rt;
			}
			template <typename CMP>
			void normal_match(const CMP& cmp, long long limited_price, order& o)
			{
				reset();
				while (0 != o.remain_quantity)
				{
					matched_record rt;
					auto it = top(cmp, rt.matched_price, rt.matched_quantity);
					if (rt)
					{
						if ((order::MARKET_PRICE != limited_price) && cmp(limited_price, rt.matched_price))
						{
							break;
						}
						auto matched_quantity = rt.matched_quantity < o.remain_quantity ? rt.matched_quantity : o.remain_quantity;
						auto matched_id = engine::get_id();
						if (_impliers.end() == it)
						{
							_local.maker_match(matched_id, matched_quantity, o.order_id);
							_local.taker_match(o, matched_id, rt.matched_price, matched_quantity, _local->order_id);
						}
						else
						{
							auto& imp = (*it->second);
							imp._leg1.maker_match(matched_id, matched_quantity, o.order_id, imp._leg2->order_id);
							imp._leg2.maker_match(matched_id, matched_quantity, o.order_id, imp._leg1->order_id);
							_local.taker_match(o, matched_id, rt.matched_price, matched_quantity, imp._leg1->order_id, imp._leg2->order_id);
						}
					}
					else
					{
						break;
					}
				}
			}
			template <typename CMP>
			void fok_match(const CMP& cmp, long long limited_price, order& o)
			{
				reset();
				auto remain_quantity = o.remain_quantity;
				std::vector<matched_record> records;
				while (0 != remain_quantity)
				{
					auto record = match(cmp, limited_price, remain_quantity);
					if (record)
					{
						remain_quantity -= record.matched_quantity;
						records.push_back(record);
					}
					else
					{
						break;
					}
				}
				if (0 != remain_quantity)
				{
					o.order_state = order::order_status_type::CANCELED_BY_FOK;
					_local._e->callback(o);
				}
				else
				{
					for (std::size_t i = 0; i < records.size(); ++i)
					{
						auto& rt = records[i];
						auto matched_id = engine::get_id();
						if (!rt.leg2)
						{
							rt.leg1.maker_match(matched_id, rt.matched_quantity, o.order_id);
							_local.taker_match(o, matched_id, rt.matched_price, rt.matched_quantity, rt.leg1->order_id);
						}
						else
						{
							rt.leg1.maker_match(matched_id, rt.matched_quantity, o.order_id, rt.leg2->order_id);
							rt.leg2.maker_match(matched_id, rt.matched_quantity, o.order_id, rt.leg1->order_id);
							_local.taker_match(o, matched_id, rt.matched_price, rt.matched_quantity, rt.leg1->order_id, rt.leg2->order_id);
						}
					}
				}
			}
		};
	private:
		static std::atomic<unsigned long long> _id;
	private:
		mutable core::spin_mutex _mutex;
		mutable mutex_set _mutex_set;
		search_order_map _odr_map;
		bid_book_type _bid_book;
		ask_book_type _ask_book;
		bid_stop_book_type _bid_stop_book;
		ask_stop_book_type _ask_stop_book;
		callback_type _callback;
		unsigned long long _mini_tick;
	public:
		inline static long long round_down(long long price, unsigned long long mini_tick)
		{
			long long mod = price % mini_tick;
			if (mod >= 0)
				return price - mod;
			else
				return price - mod - mini_tick;
		}
		inline static long long round_up(long long price, unsigned long long mini_tick)
		{
			long long mod = price % mini_tick;
			if (mod == 0)
				return mini_tick;
			if (mod > 0)
				return price + mini_tick - mod;
			else
				return price - mod;
		}
	private:
		inline static unsigned long long get_id()
		{
			return _id.fetch_add(1, std::memory_order_relaxed);
		}
		inline static void match(order& o,
				order::order_matched_type match_type,
				unsigned long long matched_id,
				long long last_match_price,
				unsigned long long last_match_quantity,
				unsigned long long last_matched_order_id,
				unsigned long long last_matched_order_id2 = 0)
		{
			o.matched_type = match_type;
			o.matched_id = matched_id;
			o.last_match_price = last_match_price;
			o.last_match_quantity = last_match_quantity;
			o.last_matched_order_id = last_matched_order_id;
			o.last_matched_order_id2 = last_matched_order_id2;
			o.remain_quantity -= o.last_match_quantity;
			if (0 == o.remain_quantity)
				o.order_state = order::order_status_type::FILLED;
			else
				o.order_state = order::order_status_type::PARTIAL_FILL;
			o.update_display();
		}
		inline static void maker_match(order& o,
				unsigned long long matched_id,
				unsigned long long last_match_quantity,
				unsigned long long last_matched_order_id,
				unsigned long long last_matched_order_id2 = 0)
		{
			match(o,
					order::order_matched_type::MAKER,
					matched_id,
					o.price,
					last_match_quantity,
					last_matched_order_id,
					last_matched_order_id2);
		}
		void callback(const order& o)
		{
			//TODO market data handler
			_callback(o);
		}
	private:
		inline void lock()
		{
			for (auto it = _mutex_set.begin(); it != _mutex_set.end(); ++it)
				(*it)->lock();
		}
		inline void unlock()
		{
			for (auto it = _mutex_set.rbegin(); it != _mutex_set.rend(); ++it)
				(*it)->unlock();
		}
	public:
		engine(callback_type&& callback, unsigned long long mini_tick = 1):
			_mutex(),
			_mutex_set(),
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
		};

		template <typename Dummy>
		struct cross_book_function_helper<Dummy, ask_book_type>
		{
			static long long get_non_cross_price(long long price, long long mini_tick)
			{
				return price - mini_tick;
			}
		};

		template <typename BookType>
		struct cross_book_function
		{
			static long long get_non_cross_price(long long price, long long mini_tick)
			{
				return cross_book_function_helper<bool, BookType>::get_non_cross_price(price, mini_tick);
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
	};
}




#endif /* MATCHING_INC_ENGINE_HPP_ */
