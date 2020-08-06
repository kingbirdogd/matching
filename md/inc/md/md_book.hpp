#ifndef MD_INC_MD_MD_BOOK_HPP_
#define MD_INC_MD_MD_BOOK_HPP_

#include <md/basic_book.hpp>
#include <md/md_implied_book.hpp>
#include <functional>
#include <vector>

namespace md
{
	class md_book
	{
	private:
		using implied_book_vec = std::vector<md_implied_book>;
	public:
		using callback = std::function<void(const book_item&)>;
	private:
		callback cb;
		long long mini_tick;
		basic_book outright;
		implied_book_vec implied_books;
	public:
		md_book
		(
			callback&& c,
			long long mtick = 1
		):
		cb(std::move(c)),
		mini_tick(mtick),
		outright(),
		implied_books()
		{
		}
		void add_implited_book
		(
			md_implied_book::implier_type bt = md_implied_book::implier_type::a_bid_b_bid,
			md_implied_book::implier_type at = md_implied_book::implier_type::a_bid_b_bid,
			implier* bi = nullptr,
			implier* ai = nullptr
		)
		{
			implied_books.push_back(md_implied_book(cb, outright, mini_tick, bt, at, bi, ai));
		}
		md_book() = delete;
		md_book(const md_book&) = default;
		md_book(md_book&&) = default;
		md_book& operator= (const md_book&) = default;
		md_book& operator= (md_book&&) = default;
		~md_book() = default;
		void recovery(callback&& cb);
		void handle_outright(const matching::order& odr);
		void handle_a(const matching::order& odr, std::size_t idx);
		void handle_b(const matching::order& odr, std::size_t idx);
		std::size_t current_idx()
		{
			return implied_books.size();
		}
	};
};



#endif /* MD_INC_MD_MD_BOOK_HPP_ */
