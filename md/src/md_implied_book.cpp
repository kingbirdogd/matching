#include <md/md_implied_book.hpp>
#include <vector>


using namespace md;

template <typename TLeg1, typename TLeg2, typename TImpliedBook>
std::vector<book_item> handle_implied
(
		TLeg1 leg1,
		TLeg2 leg2,
		TImpliedBook& implied_book,
		basic_book& outright_book,
		book_item::book_side side,
		implier* ip,
		unsigned long long mini_tick)
{
	std::vector<book_item> rt;
	TImpliedBook new_implied_book;
	auto it1 = leg1.begin();
	auto it2 = leg2.begin();
	while (it1 != leg1.end() && it2 != leg2.end())
	{
		auto implied_quantity = it1->second.quantity < it2->second.quantity ?
				it1->second.quantity : it2->second.quantity;
		auto implied_price = ip->matchd_price(it1->second.price, it2->second.price, mini_tick);
		if (side == book_item::book_side::bid)
		{
			auto it = outright_book.ask.begin();
			if (outright_book.ask.end() != it)
			{
				if (implied_price >= it->first)
				{
					implied_price = it->first - mini_tick;
				}
			}
		}
		else
		{
			auto it = outright_book.bid.begin();
			if (outright_book.bid.end() != it)
			{
				if (implied_price <= it->first)
				{
					implied_price = it->first + mini_tick;
				}
			}
		}
		book_item bitem;
		bitem.side = side;
		bitem.price = implied_price;
		bitem.quantity = implied_quantity;
		new_implied_book[implied_price] = bitem;
		it1->second.quantity -= implied_quantity;
		if (0 == it1->second.quantity)
			++it1;
		it2->second.quantity -= implied_quantity;
		if (0 == it2->second.quantity)
			++it2;
	}
	TImpliedBook new_implied_book_relace = new_implied_book;
	for (const auto& item : implied_book)
	{
		auto it = new_implied_book.find(item.first);
		if (new_implied_book.end() == it)
		{
			if (side == book_item::book_side::bid)
			{
				auto node = outright_book.reduce_bid_quantity(item.second.price, item.second.quantity);
				if (book_item::book_side::none != node.side)
				{
					rt.push_back(node);
				}
			}
			else
			{
				auto node = outright_book.reduce_ask_quantity(item.second.price, item.second.quantity);
				if (book_item::book_side::none != node.side)
				{
					rt.push_back(node);
				}
			}
		}
		else
		{
			if (it->second.quantity > item.second.quantity)
			{
				if (side == book_item::book_side::bid)
				{
					auto node = outright_book.add_bid_quantity(item.second.price, ((it->second.quantity - item.second.quantity)));
					if (book_item::book_side::none != node.side)
					{
						rt.push_back(node);
					}
				}
				else
				{
					auto node = outright_book.add_ask_quantity(item.second.price, ((it->second.quantity - item.second.quantity)));
					if (book_item::book_side::none != node.side)
					{
						rt.push_back(node);
					}
				}
			}
			else if (it->second.quantity < item.second.quantity)
			{

				if (side == book_item::book_side::bid)
				{
					auto node = outright_book.reduce_bid_quantity(item.second.price, ((item.second.quantity - it->second.quantity)));
					if (book_item::book_side::none != node.side)
					{
						rt.push_back(node);
					}
				}
				else
				{
					auto node = outright_book.reduce_ask_quantity(item.second.price, ((item.second.quantity - it->second.quantity)));
					if (book_item::book_side::none != node.side)
					{
						rt.push_back(node);
					}
				}

			}
			new_implied_book.erase(it);
		}
	}
	for (const auto& item : new_implied_book)
	{
		if (side == book_item::book_side::bid)
		{
			auto node = outright_book.add_bid_quantity(item.second.price, item.second.quantity);
			if (book_item::book_side::none != node.side)
			{
				rt.push_back(node);
			}
		}
		else
		{
			auto node = outright_book.add_ask_quantity(item.second.price, item.second.quantity);
			if (book_item::book_side::none != node.side)
			{
				rt.push_back(node);
			}
		}
	}
	implied_book = new_implied_book_relace;
	if (side == book_item::book_side::bid)
	{
		std::sort(rt.begin(), rt.end(), [](const book_item& x, const book_item& y)
		{
			return x.price > y.price;
		});
	}
	else
	{
		std::sort(rt.begin(), rt.end(), [](const book_item& x, const book_item& y)
		{
			return x.price < y.price;
		});
	}
	return rt;
}

void md_implied_book::handle_a(const matching::order& odr)
{
	auto item = a.handle_odr(odr);
	if (book_item::book_side::bid == item.side)
	{
		if (bid_implie_type == implier_type::a_bid_b_bid)
		{
			auto leg1 = a.bid;
			auto leg2 = b.bid;
			auto& implied_book = implied_bid;
			implier* ip = bid_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::bid,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		else if (bid_implie_type == implier_type::a_bid_b_ask)
		{
			auto leg1 = a.bid;
			auto leg2 = b.ask;
			auto& implied_book = implied_bid;
			implier* ip = bid_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::bid,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		if (ask_implie_type == implier_type::a_bid_b_bid)
		{
			auto leg1 = a.bid;
			auto leg2 = b.bid;
			auto& implied_book = implied_ask;
			implier* ip = ask_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::ask,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		else if (ask_implie_type == implier_type::a_bid_b_ask)
		{
			auto leg1 = a.bid;
			auto leg2 = b.ask;
			auto& implied_book = implied_ask;
			implier* ip = ask_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::ask,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
	}
	else if (book_item::book_side::ask == item.side)
	{
		if (bid_implie_type == implier_type::a_ask_b_bid)
		{
			auto leg1 = a.ask;
			auto leg2 = b.bid;
			auto& implied_book = implied_bid;
			implier* ip = bid_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::bid,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		else if (bid_implie_type == implier_type::a_ask_b_ask)
		{
			auto leg1 = a.ask;
			auto leg2 = b.ask;
			auto& implied_book = implied_bid;
			implier* ip = bid_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::bid,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		if (ask_implie_type == implier_type::a_ask_b_bid)
		{
			auto leg1 = a.ask;
			auto leg2 = b.bid;
			auto& implied_book = implied_ask;
			implier* ip = ask_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::ask,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		else if (ask_implie_type == implier_type::a_ask_b_ask)
		{
			auto leg1 = a.ask;
			auto leg2 = b.ask;
			auto& implied_book = implied_ask;
			implier* ip = ask_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::ask,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
	}
}

void md_implied_book::handle_b(const matching::order& odr)
{
	auto item = b.handle_odr(odr);
	if (book_item::book_side::bid == item.side)
	{
		if (bid_implie_type == implier_type::a_bid_b_bid)
		{
			auto leg1 = a.bid;
			auto leg2 = b.bid;
			auto& implied_book = implied_bid;
			implier* ip = bid_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::bid,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		else if (bid_implie_type == implier_type::a_ask_b_bid)
		{
			auto leg1 = a.ask;
			auto leg2 = b.bid;
			auto& implied_book = implied_bid;
			implier* ip = bid_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::bid,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		if (ask_implie_type == implier_type::a_bid_b_bid)
		{
			auto leg1 = a.bid;
			auto leg2 = b.bid;
			auto& implied_book = implied_ask;
			implier* ip = ask_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::ask,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		else if (ask_implie_type == implier_type::a_ask_b_bid)
		{
			auto leg1 = a.ask;
			auto leg2 = b.bid;
			auto& implied_book = implied_ask;
			implier* ip = ask_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::ask,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
	}
	else if (book_item::book_side::ask == item.side)
	{
		if (bid_implie_type == implier_type::a_bid_b_ask)
		{
			auto leg1 = a.bid;
			auto leg2 = b.ask;
			auto& implied_book = implied_bid;
			implier* ip = bid_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::bid,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		else if (bid_implie_type == implier_type::a_ask_b_ask)
		{
			auto leg1 = a.ask;
			auto leg2 = b.ask;
			auto& implied_book = implied_bid;
			implier* ip = bid_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::bid,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		if (ask_implie_type == implier_type::a_bid_b_ask)
		{
			auto leg1 = a.bid;
			auto leg2 = b.ask;
			auto& implied_book = implied_ask;
			implier* ip = ask_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::ask,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
		else if (ask_implie_type == implier_type::a_ask_b_ask)
		{
			auto leg1 = a.ask;
			auto leg2 = b.ask;
			auto& implied_book = implied_ask;
			implier* ip = ask_implier;
			if (ip)
			{
				auto rt = handle_implied
				(
					leg1,
					leg2,
					implied_book,
					outright,
					book_item::book_side::ask,
					ip,
					mini_tick);
				for (const auto& item : rt)
					cb(item);
			}
		}
	}
}

void md_implied_book::handle_implied_bid()
{
	if (bid_implie_type == implier_type::a_bid_b_bid)
	{
		auto leg1 = a.bid;
		auto leg2 = b.bid;
		auto& implied_book = implied_bid;
		implier* ip = bid_implier;
		if (ip)
		{
			auto rt = handle_implied
			(
				leg1,
				leg2,
				implied_book,
				outright,
				book_item::book_side::bid,
				ip,
				mini_tick);
			for (const auto& item : rt)
				cb(item);
		}
	}
	else if (bid_implie_type == implier_type::a_ask_b_bid)
	{
		auto leg1 = a.ask;
		auto leg2 = b.bid;
		auto& implied_book = implied_bid;
		implier* ip = bid_implier;
		if (ip)
		{
			auto rt = handle_implied
			(
				leg1,
				leg2,
				implied_book,
				outright,
				book_item::book_side::bid,
				ip,
				mini_tick);
			for (const auto& item : rt)
				cb(item);
		}
	}
	else if (bid_implie_type == implier_type::a_bid_b_ask)
	{
		auto leg1 = a.bid;
		auto leg2 = b.ask;
		auto& implied_book = implied_bid;
		implier* ip = bid_implier;
		if (ip)
		{
			auto rt = handle_implied
			(
				leg1,
				leg2,
				implied_book,
				outright,
				book_item::book_side::bid,
				ip,
				mini_tick);
			for (const auto& item : rt)
				cb(item);
		}
	}
	else if (bid_implie_type == implier_type::a_ask_b_ask)
	{
		auto leg1 = a.ask;
		auto leg2 = b.ask;
		auto& implied_book = implied_bid;
		implier* ip = bid_implier;
		if (ip)
		{
			auto rt = handle_implied
			(
				leg1,
				leg2,
				implied_book,
				outright,
				book_item::book_side::bid,
				ip,
				mini_tick);
			for (const auto& item : rt)
				cb(item);
		}
	}
}


void md_implied_book::handle_implied_ask()
{
	if (ask_implie_type == implier_type::a_bid_b_ask)
	{
		auto leg1 = a.bid;
		auto leg2 = b.ask;
		auto& implied_book = implied_ask;
		implier* ip = ask_implier;
		if (ip)
		{
			auto rt = handle_implied
			(
				leg1,
				leg2,
				implied_book,
				outright,
				book_item::book_side::ask,
				ip,
				mini_tick);
			for (const auto& item : rt)
				cb(item);
		}
	}
	else if (ask_implie_type == implier_type::a_ask_b_ask)
	{
		auto leg1 = a.ask;
		auto leg2 = b.ask;
		auto& implied_book = implied_ask;
		implier* ip = ask_implier;
		if (ip)
		{
			auto rt = handle_implied
			(
				leg1,
				leg2,
				implied_book,
				outright,
				book_item::book_side::ask,
				ip,
				mini_tick);
			for (const auto& item : rt)
				cb(item);
		}
	}
	else if (ask_implie_type == implier_type::a_bid_b_ask)
	{
		auto leg1 = a.bid;
		auto leg2 = b.ask;
		auto& implied_book = implied_ask;
		implier* ip = ask_implier;
		if (ip)
		{
			auto rt = handle_implied
			(
				leg1,
				leg2,
				implied_book,
				outright,
				book_item::book_side::ask,
				ip,
				mini_tick);
			for (const auto& item : rt)
				cb(item);
		}
	}
	else if (ask_implie_type == implier_type::a_ask_b_ask)
	{
		auto leg1 = a.ask;
		auto leg2 = b.ask;
		auto& implied_book = implied_ask;
		implier* ip = ask_implier;
		if (ip)
		{
			auto rt = handle_implied
			(
				leg1,
				leg2,
				implied_book,
				outright,
				book_item::book_side::ask,
				ip,
				mini_tick);
			for (const auto& item : rt)
				cb(item);
		}
	}
}



