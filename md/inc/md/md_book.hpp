#ifndef MD_INC_MD_MD_BOOK_HPP_
#define MD_INC_MD_MD_BOOK_HPP_

#include <md/basic_book.hpp>
#include <implier.hpp>
#include <functional>

namespace md
{
	class md_book
	{
	public:
		using callback = std::function<void(const book_item&)>;
	public:
		enum implier_type : unsigned long long
		{
			a_bid_b_bid = 0,
			a_bid_b_ask = 1,
			a_ask_b_bid = 2,
			a_ask_b_ask = 3
		};
	private:
		basic_book main;
		basic_book::bid_book implied_bid;
		basic_book::ask_book implied_ask;
		basic_book a;
		basic_book b;
		implier_type bid_implie_type;
		implier_type ask_implie_type;
		implier* bid_implier;
		implier* ask_implier;
	public:
		md_book
		(
			implier_type bt = implier_type::a_bid_b_bid,
			implier_type at = implier_type::a_bid_b_bid,
			implier* bi = nullptr,
			implier* ai = nullptr
		):
		main(),
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
		md_book() = delete;
		md_book(const md_book&) = default;
		md_book(md_book&&) = default;
		md_book& operator= (const md_book&) = default;
		md_book& operator= (md_book&&) = default;
		~md_book() = default;
		void recovery(callback&& cb);
	};
};



#endif /* MD_INC_MD_MD_BOOK_HPP_ */
