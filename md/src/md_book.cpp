#include <md/md_book.hpp>
#include <vector>


using namespace md;

void md_book::recovery(callback&& cb)
{
	for (auto item : outright.bid)
	{
		cb(item.second);
	}
	for (auto item : outright.ask)
	{
		cb(item.second);
	}
}

void md_book::handle_outright(const matching::order& odr)
{
	auto item = outright.handle_odr(odr);
	long long old_best_price = 0;
	if (book_item::book_side::bid == item.side)
	{
		old_best_price = outright.best_bid_price();
	}
	else if (book_item::book_side::ask == item.side)
	{
		old_best_price = outright.best_ask_price();
	}
	if (book_item::book_side::none != item.side)
	{
		cb(item);
		if (book_item::book_side::bid != item.side)
		{
			auto new_best_price = outright.best_bid_price();
			if (new_best_price < old_best_price)
			{
				for (auto& implied_book : implied_books)
				{
					implied_book.handle_implied_ask();
				}
			}
		}
		else if (book_item::book_side::ask == item.side)
		{
			auto new_best_price = outright.best_ask_price();
			if (new_best_price > old_best_price)
			{
				for (auto& implied_book : implied_books)
				{
					implied_book.handle_implied_bid();
				}
			}
		}
	}
}

void md_book::handle_a(const matching::order& odr, std::size_t idx)
{
	implied_books[idx].handle_a(odr);
}

void md_book::handle_b(const matching::order& odr, std::size_t idx)
{
	implied_books[idx].handle_b(odr);
}



