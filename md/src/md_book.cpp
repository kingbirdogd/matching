#include <md/md_book.hpp>

using namespace md;

void md_book::recovery(callback&& cb)
{
	for (auto item : main.bid)
	{
		cb(item.second);
	}
	for (auto item : main.ask)
	{
		cb(item.second);
	}
}



