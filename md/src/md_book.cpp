#include <md/md_book.hpp>

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
	if (book_item::book_side::none != item.side)
	{
		cb(item);
	}
}

void md_book::handle_a(const matching::order& odr)
{
	a.handle_odr(odr);
}

void md_book::handle_b(const matching::order& odr)
{
	b.handle_odr(odr);
}



