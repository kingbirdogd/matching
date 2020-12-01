#ifndef MATCHING_INC_ENGINE_HPP_
#define MATCHING_INC_ENGINE_HPP_

#include <time.h>
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
#include <implier.hpp>

inline static unsigned long long current()
{
		struct timespec tp;
		clock_gettime(CLOCK_REALTIME, &tp);
		unsigned long long uRt = tp.tv_sec;
		uRt *= 1000000000;
		uRt += ((unsigned long long)((unsigned long long)tp.tv_nsec));
		return uRt;
}

namespace matching
{
  auto order_set_cmp = [](order* a, order*b){ return a->order_id < b->order_id; };
	class engine
	{
	private:
		using search_order_map = std::unordered_map<unsigned long long, order>;
		using id_order_map = std::map<unsigned long long, order*>;
		//using order_set = std::unordered_set<order*>;
    //static auto order_set_cmp = [](order* a, order*b){ return a->order_id < b->order_id; };
    using order_set = std::set<order*, decltype(order_set_cmp)>;
		using bid_book_type = std::map<long long, id_order_map, std::greater<long long>>;
		using ask_book_type = std::map<long long, id_order_map, std::less<long long>>;
		using bid_stop_book_type = std::map<long long, order_set, std::less<long long>>;
		using ask_stop_book_type = std::map<long long, order_set, std::greater<long long>>;
		using callback_type = std::function<void(const order&)>;
		using mutex_set = std::set<core::spin_mutex*>;
		using callback_records = std::vector<order>;
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
			order::order_side _side;
			price_it _it1;
			price_odr _it2;
			unsigned long long _remain_quantity;
		public:
			iterator():
				_e(nullptr),
				_side(order::order_side::BUY),
				_it1(),
				_it2(),
				_remain_quantity(0)
			{
			}
			iterator(engine* e, order::order_side side):
				_e(e),
				_side(side),
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
				if(order::order_side::BUY == _side)
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
					if(order::order_side::BUY == _side)
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
					if(order::order_side::BUY == _side)
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
				if(order::order_side::BUY == _side)
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
				if(order::order_side::BUY == _side)
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
				if(order::order_side::BUY == _side)
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
				if(order::order_side::BUY == _side)
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
				if(order::order_side::BUY == _side)
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
					if(order::order_side::BUY == _side)
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
					if(order::order_side::BUY == _side)
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
				if(order::order_side::BUY == _side)
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
				else
				{
					_remain_quantity =  o.remain_quantity;
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
			long long top_price_own_side() const
      {
        long long px = order::MARKET_PRICE;
        if (_side == order::order_side::BUY)
        {
          auto it = _e->_bid_book.begin();
          if (it != _e->_bid_book.end() && it->second.size() > 0)
            px = it->second.begin()->second->price;
        }
        else if (_side == order::order_side::SELL)
        {
          auto it = _e->_ask_book.begin();
          if (it != _e->_ask_book.end() && it->second.size() > 0)
            px = it->second.begin()->second->price;
        }
        return px;
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
		class implier_base : public virtual implier
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
				order::order_side leg2_side,
				unsigned long long pips
			):
				implier(pips),
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
			unsigned long long matched_quantity;
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
		public:
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
					if (it->second->_leg1)
					{
						_local._e->_mutex_set.erase(&(it->second->_leg1._e->_mutex));
					}
					if (it->second->_leg2)
					{
						_local._e->_mutex_set.erase(&(it->second->_leg2._e->_mutex));
					}
					_impliers.erase(it);
				}
			}
			void set_implier(implier_base* imp)
			{
				unset_implier(imp);
				_impliers.emplace(imp->_priority, imp);
				if (imp->_leg1)
				{
					_local._e->_mutex_set.insert(&(imp->_leg1._e->_mutex));
				}
				if (imp->_leg2)
				{
					_local._e->_mutex_set.insert(&(imp->_leg2._e->_mutex));
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
					auto matched_price = order::MARKET_PRICE;
					if (it->second->_leg1 && it->second->_leg2)
					{
						matched_price = it->second->matchd_price(it->second->_leg1->price,
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
				}
				return price;
			}
			template <typename CMP>
			auto top(const CMP& cmp, long long& top_price, unsigned long long& top_quantity) const
			{
				auto rt = _impliers.end();
				top_price = _local.top_price();
				top_quantity = _local.top_matching_quantity();
				long long top_price_own_side = _local.top_price_own_side();
				for (auto it = _impliers.begin(); it != _impliers.end(); ++it)
				{
					auto matched_price = order::MARKET_PRICE;
					if (it->second->_leg1 && it->second->_leg2)
					{
						matched_price = it->second->matchd_price(it->second->_leg1->price,
								it->second->_leg2->price,
								_local._e->_mini_tick);
						if (cmp(matched_price, top_price_own_side)) {
						  if (_local._side == matching::order::SELL)
                matched_price = top_price_own_side - _local._e->_mini_tick;
              else if (_local._side == matching::order::BUY)
                matched_price = top_price_own_side + _local._e->_mini_tick;
            }

						if (order::MARKET_PRICE != matched_price)
						{
							//if ((order::MARKET_PRICE == top_price || cmp(matched_price, top_price)) && (order::MARKET_PRICE == top_price_own_side || cmp(top_price_own_side, matched_price)))
              if (order::MARKET_PRICE == top_price || cmp(matched_price, top_price))
							{
								top_price = matched_price;
								top_quantity = it->second->_leg1.top_matching_quantity() < it->second->_leg2.top_matching_quantity()
											? it->second->_leg1.top_matching_quantity() : it->second->_leg2.top_matching_quantity();
								rt = it;
							}
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
					if (rt.matched_quantity > quantity)
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
			template <typename CMP, typename SelfBook>
			void normal_match(const CMP& cmp, order& o, SelfBook& self, long long mini_tick)
			{
				reset();
				while (0 != o.remain_quantity)
				{
					matched_record rt;
					auto it = top(cmp, rt.matched_price, rt.matched_quantity);
					if (rt)
					{
						if ((order::MARKET_PRICE != o.price) && cmp(o.price, rt.matched_price))
						{
							break;
						}
						else if (order::order_time_condition::MAKER_ONLY == o.time_condition)
						{
							o.order_state = order::order_status_type::CANCELED_BY_MAKER_ONLY;
							_local._e->callback(o);
							return;
						}
						else if (order::order_time_condition::MAKER_ONLY_REPRICE == o.time_condition)
						{
							o.price = rt.matched_price + mini_tick;
							break;
						}
						auto matched_quantity = rt.matched_quantity < o.remain_quantity ? rt.matched_quantity : o.remain_quantity;
						auto matched_id = engine::get_id();
						if (_impliers.end() == it)
						{
							auto local_id = _local->order_id;
							_local.maker_match(matched_id, matched_quantity, o.order_id);
							_local.taker_match(o, matched_id, rt.matched_price, matched_quantity, local_id);
						}
						else
						{
							auto& imp = (*it->second);
							auto id1 = imp._leg1->order_id;
							auto id2 = imp._leg2->order_id;
							imp._leg1.maker_match(matched_id, matched_quantity, o.order_id, id2);
							imp._leg2.maker_match(matched_id, matched_quantity, o.order_id, id1);
							_local.taker_match(o, matched_id, rt.matched_price, matched_quantity, id1, id2);
						}
					}
					else
					{
						break;
					}
				}
				if (0 != o.remain_quantity)
				{
					if (order::order_time_condition::IOC == o.time_condition)
					{
						if (o.quantity == o.remain_quantity)
							o.order_state = order::order_status_type::CANCELED_ALL_BY_IOC;
						else
							o.order_state = order::order_status_type::CANCELED_PARTIAL_BY_IOC;
						_local._e->callback(o);
						return;
					}
					else if (order::order_time_condition::AUCTION == o.time_condition)
					{
						if (o.quantity == o.remain_quantity)
							o.order_state = order::order_status_type::CANCELED_ALL_BY_AUCTION;
						else
							o.order_state = order::order_status_type::CANCELED_PARTIAL_BY_AUCTION;
						_local._e->callback(o);
						return;
					}
					if (order::MARKET_PRICE == o.price)
					{
						if (order::order_side::BUY == _local._side)
						{
							o.price = _local._e->best_bid();
						}
						else
						{
							o.price = _local._e->best_ask();
						}
						if (order::MARKET_PRICE == o.price)
						{
							o.order_state = order::order_status_type::CANCELED_BY_MARKET_ORDER_NOTHING_MATCH;
							_local._e->callback(o);
							return;
						}
					}
					self[o.price][o.order_id] = &((_local._e->_odr_map.emplace(o.order_id, o).first)->second);
					if (o.remain_quantity == o.quantity)
					{
						_local._e->callback(o);
					}
				}
			}
			template <typename CMP>
			void fok_match(const CMP& cmp, order& o)
			{
				reset();
				auto remain_quantity = o.remain_quantity;
				std::vector<matched_record> records;
				while (0 != remain_quantity)
				{
					auto record = match(cmp, o.price, remain_quantity);
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
					return;
				}
				else
				{
					for (std::size_t i = 0; i < records.size(); ++i)
					{
						auto& rt = records[i];
						auto matched_id = engine::get_id();
						if (!rt.leg2)
						{
							auto id1 = rt.leg1->order_id;
							rt.leg1.maker_match(matched_id, rt.matched_quantity, o.order_id);
							_local.taker_match(o, matched_id, rt.matched_price, rt.matched_quantity, id1);
						}
						else
						{
							auto id1 = rt.leg1->order_id;
							auto id2 = rt.leg2->order_id;
							rt.leg1.maker_match(matched_id, rt.matched_quantity, o.order_id, id2);
							rt.leg2.maker_match(matched_id, rt.matched_quantity, o.order_id, id1);
							_local.taker_match(o, matched_id, rt.matched_price, rt.matched_quantity, id1, id2);
						}
					}
				}
			}
		};
	private:
		static std::atomic<unsigned long long> _id;
		static unsigned long long _node_id;
	private:
		mutable core::spin_mutex _mutex;
		mutable mutex_set _mutex_set;
		search_order_map _odr_map;
		bid_book_type _bid_book;
		ask_book_type _ask_book;
		bid_stop_book_type _bid_stop_book;
		ask_stop_book_type _ask_stop_book;
		matcher _bid_book_matcher;
		matcher _ask_book_matcher;
		callback_records _cb_records;
		callback_type _callback;
		long long _mini_tick;
		bool _is_auction;
	public:
		inline static void set_node_id(unsigned long long node_id)
		{
			_node_id = (node_id << 57);
		}
	private:
		inline static unsigned long long get_id()
		{
			return (_id.fetch_add(1, std::memory_order_relaxed) | _node_id);
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
		void callback(order& o)
		{
			o.timestamp_epoch_ms = current() / 1000000;
			if (_is_auction)
			{
				_cb_records.push_back(o);
			}
			else
			{
				_callback(o);
			}
		}
	private:
		long long best_bid()
		{
			_bid_book_matcher.reset();
			return _bid_book_matcher.top_price(_bid_book.key_comp());
		}
		long long best_ask()
		{
			_ask_book_matcher.reset();
			return _ask_book_matcher.top_price(_ask_book.key_comp());
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

		void erase_buy_stop(order& o)
		{
			auto ori_it_price = _bid_stop_book.find(o.buy_stop_trigger_price);
			ori_it_price->second.erase(&o);
			if (ori_it_price->second.empty())
			{
				_bid_stop_book.erase(ori_it_price);
			}
		}

		void erase_sell_stop(order& o)
		{
			auto ori_it_price = _ask_stop_book.find(o.sell_stop_trigger_price);
			ori_it_price->second.erase(&o);
			if (ori_it_price->second.empty())
			{
				_ask_stop_book.erase(ori_it_price);
			}
		}
	private:
		void trigger_buy_stop()
		{
			long long mini_tick = _mini_tick;
			mini_tick *= -1;
			std::vector<order*> odr_list;
			do
			{
				odr_list.clear();
				auto best = best_ask();
				for (auto it = _bid_stop_book.begin(); it != _bid_stop_book.end();)
				{
					if ((best != order::MARKET_PRICE) && (best >= it->first))
					{
						for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
						{
							odr_list.push_back(*it2);
						}
						it = _bid_stop_book.erase(it);
					}
					else
					{
						++it;
					}
				}
				for (std::size_t i = 0; i < odr_list.size(); ++i)
				{
					auto& o = (*odr_list[i]);
					o.price = o.buy_stop_limit_price;
					if (order::order_time_condition::FOK == o.time_condition)
					{
						_ask_book_matcher.fok_match(_ask_book.key_comp(), o);
					}
					else
					{
						_ask_book_matcher.normal_match(_ask_book.key_comp(), o, _bid_book, mini_tick);
					}
				}
			}
			while(!odr_list.empty());
		}

		void trigger_sell_stop()
		{
			long long mini_tick = _mini_tick;
			std::vector<order*> odr_list;
			do
			{
				odr_list.clear();
				auto best = best_bid();
				for (auto it = _ask_stop_book.begin(); it != _ask_stop_book.end();)
				{
					if ((best != order::MARKET_PRICE) && (best <= it->first))
					{
						for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
						{
							odr_list.push_back(*it2);
						}
						it = _ask_stop_book.erase(it);
					}
					else
					{
						++it;
					}
				}
				for (std::size_t i = 0; i < odr_list.size(); ++i)
				{
					auto& o = (*odr_list[i]);
					o.price = o.sell_stop_limit_price;
					if (order::order_time_condition::FOK == o.time_condition)
					{
						_bid_book_matcher.fok_match(_bid_book.key_comp(), o);
					}
					else
					{
						_bid_book_matcher.normal_match(_bid_book.key_comp(), o, _ask_book, mini_tick);
					}
				}
			}
			while(!odr_list.empty());
		}
	private:
		inline void init_new_order(order& o)
		{
			if (0 == o.order_id)
			{
				o.order_id = get_id();
			}
			o.remain_quantity = o.quantity;
			o.order_state = order::order_status_type::OPEN;
		}

		inline void handle_new(order& o)
		{
			long long mini_tick = _mini_tick;
			if (order::order_action_type::NEW == o.order_action)
			{
				o.order_id = 0;
			}
			if (0 == o.quantity)
			{
				o.order_state = order::order_status_type::REJECT_QUANTITY_ZERO;
				o.remain_quantity = 0;
				callback(o);
				return;
			}
			if (o.display_quantity > o.quantity)
			{
				o.order_state = order::order_status_type::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY;
				o.remain_quantity = 0;
				callback(o);
				return;
			}
			if (order::order_side::BUY == o.side)
			{
				init_new_order(o);
				if (order::order_type::MARKET == o.type)
				{
//					if (order::MARKET_PRICE != o.price)
//					{
//						o.price = best_ask() + o.price;
//					}
				}
				else if (order::MARKET_PRICE == o.price)
				{
					o.order_state = order::order_status_type::REJECT_LIMITE_ORDER_WITH_MARKET_PRICE;
					o.remain_quantity = 0;
					callback(o);
					return;
				}
				if (order::order_time_condition::FOK == o.time_condition)
				{
					_ask_book_matcher.fok_match(_ask_book.key_comp(), o);
				}
				else
				{
					_ask_book_matcher.normal_match(_ask_book.key_comp(), o, _bid_book, mini_tick * -1);
				}
				//if (o.remain_quantity != o.quantity)
        if (o.remain_quantity > 0)
				{
          trigger_sell_stop();
          trigger_buy_stop();
        }
				return;
			}
			else if (order::order_side::SELL == o.side)
			{
				init_new_order(o);
				if (order::order_type::MARKET == o.type)
				{
//					if (order::MARKET_PRICE != o.price)
//					{
//						o.price = best_bid() - o.price;
//					}
				}
				else if (order::MARKET_PRICE == o.price)
				{
					o.order_state = order::order_status_type::REJECT_LIMITE_ORDER_WITH_MARKET_PRICE;
					o.remain_quantity = 0;
					callback(o);
					return;
				}
				if (order::order_time_condition::FOK == o.time_condition)
				{
					_bid_book_matcher.fok_match(_bid_book.key_comp(), o);
				}
				else
				{
					_bid_book_matcher.normal_match(_bid_book.key_comp(), o, _ask_book, mini_tick);
				}
				//if (o.remain_quantity != o.quantity)
        if (o.remain_quantity > 0)
				{
					trigger_sell_stop();
          trigger_buy_stop();
				}
				return;
			}
			else if (order::order_side::BUY_STOP == o.side)
			{
				o.price = order::STOP_PRICE;
				auto best_a = best_ask();
				if (order::order_type::MARKET == o.type)
				{
//					if (order::MARKET_PRICE != o.buy_stop_limit_price)
//					{
//						auto base_price = o.buy_stop_trigger_price;
//						if (order::MARKET_PRICE != best_a)
//						{
//							if (best_a > base_price)
//								base_price = best_a;
//						}
//						o.buy_stop_limit_price = base_price + o.buy_stop_limit_price;
//					}
				}
				if (order::MARKET_PRICE == o.buy_stop_limit_price)
				{
					o.order_state = order::order_status_type::REJECT_LIMITE_ORDER_WITH_MARKET_PRICE;
					o.remain_quantity = 0;
					callback(o);
					return;
				}
				if (o.buy_stop_trigger_price > o.buy_stop_limit_price)
				{
					o.order_state = order::order_status_type::REJECT_BUY_STOP_TRIGGER_LARGE_THAN_STOP_LIMIT;
					o.remain_quantity = 0;
					callback(o);
					return;
				}
				init_new_order(o);
				if (order::MARKET_PRICE != best_a && o.buy_stop_trigger_price <= best_a)
				{
					o.price = o.buy_stop_limit_price;
					if (order::order_time_condition::FOK == o.time_condition)
					{
						_ask_book_matcher.fok_match(_ask_book.key_comp(), o);
					}
					else
					{
						_ask_book_matcher.normal_match(_ask_book.key_comp(), o, _bid_book, mini_tick * -1);
					}
					//if (o.remain_quantity != o.quantity)
          if (o.remain_quantity > 0)
					{
            trigger_sell_stop();
            trigger_buy_stop();
          }
				}
				else
				{
				  auto [it, inserted] = _bid_stop_book.insert({o.buy_stop_trigger_price, order_set{order_set_cmp}});
				  it->second.insert(&((_odr_map.emplace(o.order_id, o).first)->second));
					//_bid_stop_book[o.buy_stop_trigger_price].insert(&((_odr_map.emplace(o.order_id, o).first)->second));
					callback(o);
				}
				return;
			}
			else if (order::order_side::SELL_STOP == o.side)
			{
				o.price = order::STOP_PRICE;
				auto best_b = best_bid();
				if (order::order_type::MARKET == o.type)
				{
//					if (order::MARKET_PRICE != o.sell_stop_limit_price)
//					{
//						auto base_price = o.sell_stop_trigger_price;
//						if (order::MARKET_PRICE != best_b)
//						{
//							if (best_b < base_price)
//								base_price = best_b;
//						}
//						o.sell_stop_limit_price = base_price + o.sell_stop_limit_price;
//					}
				}
				if (order::MARKET_PRICE == o.sell_stop_limit_price)
				{
					o.order_state = order::order_status_type::REJECT_LIMITE_ORDER_WITH_MARKET_PRICE;
					o.remain_quantity = 0;
					callback(o);
					return;
				}
				if (o.sell_stop_trigger_price < o.sell_stop_limit_price)
				{
					o.order_state = order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMIT;
					o.remain_quantity = 0;
					callback(o);
					return;
				}
				init_new_order(o);
				if (order::MARKET_PRICE != best_b && o.sell_stop_trigger_price >= best_b)
				{
					o.price = o.sell_stop_limit_price;
					if (order::order_time_condition::FOK == o.time_condition)
					{
						_bid_book_matcher.fok_match(_bid_book.key_comp(), o);
					}
					else
					{
						_bid_book_matcher.normal_match(_bid_book.key_comp(), o, _ask_book, mini_tick);
					}
					//if (o.remain_quantity != o.quantity)
          if (o.remain_quantity > 0)
					{
            trigger_sell_stop();
            trigger_buy_stop();
          }
				}
				else
				{
				  auto [it, inserted] = _ask_stop_book.insert({o.sell_stop_trigger_price, order_set{order_set_cmp}});
				  it->second.insert(&((_odr_map.emplace(o.order_id, o).first)->second));
					//_ask_stop_book[o.sell_stop_trigger_price].insert(&((_odr_map.emplace(o.order_id, o).first)->second));
					callback(o);
				}
				return;
			}
			else if (order::order_side::BUY_SELL_STOP == o.side)
			{
				o.price = order::STOP_PRICE;
				auto best_a = best_ask();
				auto best_b = best_bid();
				if (order::order_type::MARKET == o.type)
				{
//					if (order::MARKET_PRICE != o.buy_stop_limit_price)
//					{
//						auto base_price = o.buy_stop_trigger_price;
//						if (order::MARKET_PRICE != best_a)
//						{
//							if (best_a > base_price)
//								base_price = best_a;
//						}
//						o.buy_stop_limit_price = base_price + o.buy_stop_limit_price;
//					}
//					if (order::MARKET_PRICE != o.sell_stop_limit_price)
//					{
//						auto base_price = o.sell_stop_trigger_price;
//						if (order::MARKET_PRICE != best_b)
//						{
//							if (best_b < base_price)
//								base_price = best_b;
//						}
//						o.sell_stop_limit_price = base_price + o.sell_stop_limit_price;
//					}
				}
				if (order::MARKET_PRICE == o.buy_stop_limit_price)
				{
					o.order_state = order::order_status_type::REJECT_LIMITE_ORDER_WITH_MARKET_PRICE;
					o.remain_quantity = 0;
					callback(o);
					return;
				}
				if (o.buy_stop_trigger_price > o.buy_stop_limit_price)
				{
					o.order_state = order::order_status_type::REJECT_BUY_STOP_TRIGGER_LARGE_THAN_STOP_LIMIT;
					o.remain_quantity = 0;
					callback(o);
					return;
				}
				if (order::MARKET_PRICE == o.sell_stop_limit_price)
				{
					o.order_state = order::order_status_type::REJECT_LIMITE_ORDER_WITH_MARKET_PRICE;
					o.remain_quantity = 0;
					callback(o);
					return;
				}
				if (o.sell_stop_trigger_price < o.sell_stop_limit_price)
				{
					o.order_state = order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMIT;
					o.remain_quantity = 0;
					callback(o);
					return;
				}
				init_new_order(o);
				if (order::MARKET_PRICE != best_a && o.buy_stop_trigger_price <= best_a)
				{
					o.price = o.buy_stop_limit_price;
					if (order::order_time_condition::FOK == o.time_condition)
					{
						_ask_book_matcher.fok_match(_ask_book.key_comp(), o);
					}
					else
					{
						_ask_book_matcher.normal_match(_ask_book.key_comp(), o, _bid_book, mini_tick * -1);
					}
					//if (o.remain_quantity != o.quantity)
          if (o.remain_quantity > 0)
					{
            trigger_sell_stop();
            trigger_buy_stop();
          }
				}
				else if (order::MARKET_PRICE == best_b || o.sell_stop_trigger_price >= best_b)
				{
					o.price = o.sell_stop_limit_price;
					if (order::order_time_condition::FOK == o.time_condition)
					{
						_bid_book_matcher.fok_match(_bid_book.key_comp(), o);
					}
					else
					{
						_bid_book_matcher.normal_match(_bid_book.key_comp(), o, _ask_book, mini_tick);
					}
					//if (o.remain_quantity != o.quantity)
          if (o.remain_quantity > 0)
					{
            trigger_sell_stop();
            trigger_buy_stop();
          }
				}
				else
				{
					auto ptr = &((_odr_map.emplace(o.order_id, o).first)->second);
          auto [it_ask, inserted_ask] = _ask_stop_book.insert({o.sell_stop_trigger_price, order_set{order_set_cmp}});
          auto [it_buy, inserted_buy] = _bid_stop_book.insert({o.buy_stop_trigger_price,  order_set{order_set_cmp}});
          it_buy->second.insert(ptr);
          it_ask->second.insert(ptr);
					//_bid_stop_book[o.buy_stop_trigger_price].insert(ptr);
					//_ask_stop_book[o.sell_stop_trigger_price].insert(ptr);
					callback(o);
				}
				return;
			}
			else
			{
				o.order_state = order::order_status_type::REJECT_UNKNOW_ORDER_ACTION;
				o.remain_quantity = 0;
				callback(o);
				return;
			}
		}

		inline bool handle_cancel(order& o)
		{
			auto it = _odr_map.find(o.order_id);
			if (_odr_map.end() == it)
			{
				o.remain_quantity = 0;
				if (order::order_action_type::CANCEL == o.order_action)
				{
					o.order_state = order::order_status_type::REJECT_CANCEL_ORDER_ID_NOT_FOUND;
					callback(o);
				}
				else if (order::order_action_type::AMEND == o.order_action)
				{
					o.order_state = order::order_status_type::REJECT_AMEND_ORDER_ID_NOT_FOUND;
				}
				return false;
			}
			auto& ori_odr = it->second;
			if (order::order_action_type::CANCEL == o.order_action)
			{
				//ori_odr.client_order_id = o.client_order_id;
				ori_odr.order_state = order::order_status_type::CANCELED_BY_USER;
				callback(ori_odr);
			}
			else if (order::order_action_type::AMEND == o.order_action)
			{
				ori_odr.order_state = order::order_status_type::CANCELED_BY_AMEND;
				callback(ori_odr);
			}
			if (order::order_side::BUY == ori_odr.side)
			{
				bool trigger = false;
				auto it = _bid_book.begin();
				if ((it != _bid_book.end()) && (1 == it->second.size()) && (ori_odr.price == it->first))
				{
					trigger = true;
				}
				erase_from_normal_book(_bid_book, ori_odr);
				if (trigger)
				{
					trigger_sell_stop();
				}
			}
			else if (order::order_side::SELL == ori_odr.side)
			{
				bool trigger = false;
				auto it = _ask_book.begin();
				if ((it != _ask_book.end()) && (1 == it->second.size()) && (ori_odr.price == it->first))
				{
					trigger = true;
				}
				erase_from_normal_book(_ask_book, ori_odr);
				if (trigger)
				{
					trigger_buy_stop();
				}
			}
			else if (order::order_side::BUY_STOP == ori_odr.side)
			{
				if (order::STOP_PRICE == ori_odr.price)
				{
					erase_buy_stop(ori_odr);
				}
				else
				{
					bool trigger = false;
					auto it = _bid_book.begin();
					if ((it != _bid_book.end()) && (1 == it->second.size()) && (ori_odr.price == it->first))
					{
						trigger = true;
					}
					erase_from_normal_book(_bid_book, ori_odr);
					if (trigger)
					{
						trigger_sell_stop();
					}
				}
			}
			else if (order::order_side::SELL_STOP == ori_odr.side)
			{
				if (order::STOP_PRICE == ori_odr.price)
				{
					erase_sell_stop(ori_odr);
				}
				else
				{
					bool trigger = false;
					auto it = _ask_book.begin();
					if ((it != _ask_book.end()) && (1 == it->second.size()) && (ori_odr.price == it->first))
					{
						trigger = true;
					}
					erase_from_normal_book(_ask_book, ori_odr);
					if (trigger)
					{
						trigger_buy_stop();
					}
				}
			}
			else
			{
				if (order::STOP_PRICE == ori_odr.price)
				{
					erase_buy_stop(ori_odr);
					erase_sell_stop(ori_odr);
				}
				else if (ori_odr.price == ori_odr.buy_stop_limit_price)
				{
					bool trigger = false;
					auto it = _bid_book.begin();
					if ((it != _bid_book.end()) && (1 == it->second.size()) && (ori_odr.price == it->first))
					{
						trigger = true;
					}
					erase_from_normal_book(_bid_book, ori_odr);
					if (trigger)
					{
						trigger_sell_stop();
					}
				}
				else
				{
					bool trigger = false;
					auto it = _ask_book.begin();
					if ((it != _ask_book.end()) && (1 == it->second.size()) && (ori_odr.price == it->first))
					{
						trigger = true;
					}
					erase_from_normal_book(_ask_book, ori_odr);
					if (trigger)
					{
						trigger_buy_stop();
					}
				}
			}
			_odr_map.erase(it);
			return true;
		}

		//can keep the priority if just reduce quantity
		inline void handle_amend(order& o)
		{
			if (0 == o.quantity)
			{
				o.order_state = order::order_status_type::REJECT_QUANTITY_ZERO;
				o.remain_quantity = 0;
				callback(o);
				return;
			}
			if (o.display_quantity > o.quantity)
			{
				o.order_state = order::order_status_type::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY;
				o.remain_quantity = 0;
				callback(o);
				return;
			}
			auto it = _odr_map.find(o.order_id);
			if (_odr_map.end() == it)
			{
				o.order_state = order::order_status_type::REJECT_AMEND_ORDER_ID_NOT_FOUND;
				o.remain_quantity = 0;
				callback(o);
				return;
			}
			auto& ori_odr = it->second;
			ori_odr.order_action = o.order_action;
			auto client_order_id = ori_odr.client_order_id;
			if (order::can_amend(o, ori_odr))
			{
				//ori_odr.client_order_id = o.client_order_id;
				ori_odr.quantity = o.quantity;
				ori_odr.display_quantity = o.display_quantity;
				ori_odr.remain_quantity = ori_odr.quantity;
				ori_odr.request_id = o.request_id;
				callback(ori_odr);
				return;
			}
			else if (handle_cancel(o))
			{
				o.client_order_id = client_order_id;
				o.order_id = 0;
				handle_new(o);
			}
		}
	public:
		engine(callback_type&& callback, long long mini_tick = 1):
			_mutex(),
			_mutex_set(),
			_odr_map(),
			_bid_book(),
			_ask_book(),
			_bid_stop_book(),
			_ask_stop_book(),
			_bid_book_matcher(this, order::order_side::SELL),
			_ask_book_matcher(this, order::order_side::BUY),
			_callback(std::move(callback)),
			_mini_tick(mini_tick),
			_is_auction(false)
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
			_bid_book_matcher(this, order::order_side::SELL),
			_ask_book_matcher(this, order::order_side::BUY),
			_cb_records(),
			_callback(e._callback),
			_mini_tick(e._mini_tick),
			_is_auction(e._is_auction)
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
			_bid_book_matcher(this, order::order_side::SELL),
			_ask_book_matcher(this, order::order_side::BUY),
			_cb_records(std::move(e._cb_records)),
			_callback(std::move(e._callback)),
			_mini_tick(e._mini_tick),
			_is_auction(e._is_auction)
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
			_cb_records = e._cb_records;
			_callback = e._callback;
			_mini_tick = e._mini_tick;
			_is_auction = e._is_auction;
			return * this;
		}
		engine& operator= (engine&& e)
		{
			_odr_map = std::move(e._odr_map);
			_bid_book = std::move(e._bid_book);
			_ask_book = std::move(e._ask_book);
			_bid_stop_book = std::move(e._bid_stop_book);
			_ask_stop_book = std::move(e._ask_stop_book);
			_cb_records = std::move(e._cb_records);
			_callback = std::move(e._callback);
			_mini_tick = e._mini_tick;
			_is_auction = e._is_auction;
			return * this;
		}
		~engine() = default;
	public:
		void handle(order& o)
		{
			lock();
			if (order::order_time_condition::AUCTION == o.time_condition)
			{
				if (order::order_side::BUY == o.side || order::order_side::SELL == o.side)
				{
					_is_auction = true;
				}
				else
				{
					o.order_state = order::order_status_type::REJECT_AUCTION_SUPPORT_BUY_SELL_ONLY;
					_callback(o);
					return;
				}
			}
			if (order::order_action_type::NEW == o.order_action)
			{
				handle_new(o);
			}
			else if (order::order_action_type::CANCEL == o.order_action)
			{
				handle_cancel(o);
			}
			else if (order::order_action_type::AMEND == o.order_action)
			{
				handle_amend(o);
			}
			else if (order::order_action_type::DUMP == o.order_action)
			{
				for (const auto& item: _odr_map)
				{
					auto out_odr = item.second;
					if (order::order_action_type::NEW == out_odr.order_action)
					{
						out_odr.order_action = order::order_action_type::INSERT_NEW;
					}
					else if (order::order_action_type::AMEND == out_odr.order_action)
					{
						out_odr.order_action = order::order_action_type::INSERT_AMEND;
					}
					_callback(out_odr);
				}
				order end_odr;
				end_odr.order_action = order::order_action_type::DUMP_END;
				_callback(end_odr);
			}
			else
			{
				if (order::order_action_type::INSERT_NEW == o.order_action)
				{
					o.order_action = order::order_action_type::NEW;
				}
				else if (order::order_action_type::INSERT_AMEND == o.order_action)
				{
					o.order_action = order::order_action_type::AMEND;
				}
				if (order::order_side::BUY == o.side)
				{
					_bid_book[o.price][o.order_id] = &((_odr_map.emplace(o.order_id, o).first)->second);
				}
				else if (order::order_side::SELL == o.side)
				{
					_ask_book[o.price][o.order_id] = &((_odr_map.emplace(o.order_id, o).first)->second);
				}
				else if (order::order_side::BUY_STOP == o.side)
				{
				  if (o.price == o.buy_stop_limit_price)
            _bid_book[o.price][o.order_id] = &((_odr_map.emplace(o.order_id, o).first)->second);
          else {
            o.price = order::STOP_PRICE;
            auto [it_buy, inserted_buy] = _bid_stop_book.insert({o.buy_stop_trigger_price,  order_set{order_set_cmp}});
            it_buy->second.insert(&((_odr_map.emplace(o.order_id, o).first)->second));
            //_bid_stop_book[o.buy_stop_trigger_price].insert(&((_odr_map.emplace(o.order_id, o).first)->second));
          }
				}
				else if (order::order_side::SELL_STOP == o.side)
				{
          if (o.price == o.sell_stop_limit_price)
            _ask_book[o.price][o.order_id] = &((_odr_map.emplace(o.order_id, o).first)->second);
          else {
            o.price = order::STOP_PRICE;
            auto [it_buy, inserted_buy] = _bid_stop_book.insert({o.buy_stop_trigger_price,  order_set{order_set_cmp}});
            it_buy->second.insert(&((_odr_map.emplace(o.order_id, o).first)->second));
            //_bid_stop_book[o.buy_stop_trigger_price].insert(&((_odr_map.emplace(o.order_id, o).first)->second));
          }
				}
				else if (order::order_side::BUY_SELL_STOP == o.side)
				{
					auto ptr = &((_odr_map.emplace(o.order_id, o).first)->second);
          auto [it_ask, inserted_ask] = _ask_stop_book.insert({o.sell_stop_trigger_price, order_set{order_set_cmp}});
          auto [it_buy, inserted_buy] = _bid_stop_book.insert({o.buy_stop_trigger_price,  order_set{order_set_cmp}});
          it_buy->second.insert(ptr);
          it_ask->second.insert(ptr);
					//_bid_stop_book[o.buy_stop_trigger_price].insert(ptr);
					//_ask_stop_book[o.sell_stop_trigger_price].insert(ptr);
				}
				_callback(o);
			}
			if (_is_auction)
			{
				_is_auction = false;
				long long last_idx = -1;
				long long last_match_price = 0;
				for (long long i = _cb_records.size() - 1; i >= 0; --i)
				{
					const auto& o = _cb_records[i];
					if (order::order_status_type::CANCELED_ALL_BY_AUCTION != o.order_state
							&& order::order_status_type::CANCELED_PARTIAL_BY_AUCTION != o.order_state
							&& -1 == last_idx)
					{
						last_idx = i;
						last_match_price = o.last_match_price;
					}
				}
				if (-1 != last_idx)
				{
					if (order::order_side::BUY == o.side)
					{
						if (last_match_price < 0)
						{
							last_match_price = 0;
						}
						if (0 != o.remain_quantity)
						{
							last_match_price = o.buy_stop_limit_price;
						}
					}
					else if (order::order_side::SELL == o.side)
					{
						if (last_match_price > 0)
						{
							last_match_price = 0;
						}
						if (0 != o.remain_quantity)
						{
							last_match_price = o.sell_stop_limit_price;
						}
					}
					for (std::size_t i = 0; i < _cb_records.size(); ++i)
					{
						_cb_records[i].last_match_price = last_match_price;
					}
				}
				for (std::size_t i = 0; i < _cb_records.size(); ++i)
				{
					_callback(_cb_records[i]);
				}
				_cb_records.resize(0);
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
		inline void set_bid_implier(implier_base* imp)
		{
			_bid_book_matcher.set_implier(imp);
		}
		inline void unset_bid_implier(implier_base* imp)
		{
			_bid_book_matcher.unset_implier(imp);
		}
		inline void set_ask_implier(implier_base* imp)
		{
			_ask_book_matcher.set_implier(imp);
		}
		inline void unset_ask_implier(implier_base* imp)
		{
			_ask_book_matcher.unset_implier(imp);
		}
	};
}




#endif /* MATCHING_INC_ENGINE_HPP_ */
