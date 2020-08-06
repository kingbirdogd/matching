#ifndef MD_INC_MD_MD_IMPLIED_BOOK_HPP_
#define MD_INC_MD_MD_IMPLIED_BOOK_HPP_

#include <md/basic_book.hpp>
#include <implier.hpp>
#include <functional>

namespace md
{
	class md_implied_book
	{
	public:
		using callback = std::function<void(const book_item&)>;
	public:
		enum implier_type : unsigned long long
		{
			a_bid_b_bid = 0,
			a_bid_b_ask = 1,
			a_ask_b_bid = 2,
			a_ask_b_ask = 3,
			a_none_b_none = 4
		};
	private:
		callback& cb;
		basic_book& outright;
		long long mini_tick;
		basic_book::bid_book implied_bid;
		basic_book::ask_book implied_ask;
		basic_book a;
		basic_book b;
		implier_type bid_implie_type;
		implier_type ask_implie_type;
		implier* bid_implier;
		implier* ask_implier;
	public:
		md_implied_book
		(
			callback& c,
			basic_book& out,
			long long mtick = 1,
			implier_type bt = implier_type::a_bid_b_bid,
			implier_type at = implier_type::a_bid_b_bid,
			implier* bi = nullptr,
			implier* ai = nullptr
		):
		cb(c),
		outright(out),
		mini_tick(mtick),
		implied_bid(),
		implied_ask(),
		a(),
		b(),
		bid_implie_type(bt),
		ask_implie_type(at),
		bid_implier(bi),
		ask_implier(ai)
		{
		}
		md_implied_book() = delete;
		md_implied_book(const md_implied_book& ib) = delete;
		md_implied_book(md_implied_book&& ib):
			cb(ib.cb),
			outright(ib.outright),
			mini_tick(ib.mini_tick),
			implied_bid(std::move(ib.implied_bid)),
			implied_ask(std::move(ib.implied_ask)),
			a(std::move(ib.a)),
			b(std::move(ib.b)),
			bid_implie_type(ib.bid_implie_type),
			ask_implie_type(ib.ask_implie_type),
			bid_implier(ib.bid_implier),
			ask_implier(ib.ask_implier)
		{
		}
		md_implied_book& operator= (const md_implied_book&) = delete;
		md_implied_book& operator= (md_implied_book&& ib)
		{
			cb = ib.cb;
			outright = ib.outright;
			mini_tick = ib.mini_tick;
			implied_bid = std::move(ib.implied_bid);
			implied_ask = std::move(ib.implied_ask);
			a = std::move(ib.a);
			b = std::move(ib.b);
			bid_implie_type = ib.bid_implie_type;
			ask_implie_type = ib.ask_implie_type;
			bid_implier = ib.bid_implier;
			ask_implier = ib.ask_implier;
			return *this;
		}
		~md_implied_book() = default;
		void handle_a(const matching::order& odr);
		void handle_b(const matching::order& odr);
	};
};


#endif /* MD_INC_MD_MD_IMPLIED_BOOK_HPP_ */
