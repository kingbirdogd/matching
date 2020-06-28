#include <md/basic_book.hpp>

using namespace md;

book_item basic_book::add_bid_quantity(long long price, unsigned long long quantity)
{
	auto& item = bid[price];
	item.price = price;
	item.side = book_item::book_side::bid;
	item.quantity += quantity;
	return item;
}

book_item basic_book::reduce_bid_quantity(long long price, unsigned long long quantity)
{
	auto it = bid.find(price);
	it->second.price = price;
	it->second.side = book_item::book_side::bid;
	it->second.quantity -= quantity;
	auto rt = it->second;
	if (0 == it->second.quantity)
		bid.erase(it);
	return rt;
}

book_item basic_book::add_ask_quantity(long long price, unsigned long long quantity)
{
	auto& item = ask[price];
	item.price = price;
	item.side = book_item::book_side::ask;
	item.quantity += quantity;
	return item;
}

book_item basic_book::reduce_ask_quantity(long long price, unsigned long long quantity)
{
	auto it = ask.find(price);
	it->second.price = price;
	it->second.side = book_item::book_side::ask;
	it->second.quantity -= quantity;
	auto rt = it->second;
	if (0 == it->second.quantity)
		ask.erase(it);
	return rt;
}

book_item basic_book::handle_odr(const matching::order& odr)
{
	if (0 == odr.price)
		return book_item();
	else
	{
		auto calc_remain_quantity = odr.remain_quantity;
		if (matching::order::order_status_type::OPEN != odr.order_state
				&& matching::order::order_status_type::PARTIAL_FILL != odr.order_state)
		{
			calc_remain_quantity = 0;
		}
		if (matching::order::order_side::BUY == odr.side
				|| matching::order::order_side::BUY_STOP == odr.side
				|| (matching::order::order_side::BUY_SELL_STOP == odr.side
						&& odr.price >= odr.buy_stop_trigger_price))
		{
			auto it = orders.find(odr.order_id);
			if (orders.end() == it)
			{
				if (0 != calc_remain_quantity)
				{
					orders[odr.order_id] = odr;
					return add_bid_quantity(odr.price, calc_remain_quantity);
				}
				else
				{
					return book_item();
				}
			}
			else
			{
				if (it->second.remain_quantity < calc_remain_quantity)
				{
					auto diff = calc_remain_quantity - it->second.remain_quantity;
					it->second.remain_quantity = calc_remain_quantity;
					return add_bid_quantity(odr.price, diff);
				}
				else if (it->second.remain_quantity > calc_remain_quantity)
				{
					auto diff = it->second.remain_quantity - calc_remain_quantity;
					it->second.remain_quantity = calc_remain_quantity;
					return reduce_bid_quantity(odr.price, diff);
				}
				else
				{
					return book_item();
				}
				if (0 == it->second.remain_quantity)
				{
					orders.erase(it);
				}
			}
		}
		else if (matching::order::order_side::SELL == odr.side
				|| matching::order::order_side::SELL_STOP == odr.side
				|| (matching::order::order_side::BUY_SELL_STOP == odr.side
						&& odr.price <= odr.sell_stop_trigger_price))
		{
			auto it = orders.find(odr.order_id);
			if (orders.end() == it)
			{
				if (0 != calc_remain_quantity)
				{
					orders[odr.order_id] = odr;
					return add_ask_quantity(odr.price, calc_remain_quantity);
				}
				else
				{
					return book_item();
				}
			}
			else
			{
				if (it->second.remain_quantity < calc_remain_quantity)
				{
					auto diff = calc_remain_quantity - it->second.remain_quantity;
					it->second.remain_quantity = calc_remain_quantity;
					return add_ask_quantity(odr.price, diff);
				}
				else if (it->second.remain_quantity > calc_remain_quantity)
				{
					auto diff = it->second.remain_quantity - calc_remain_quantity;
					it->second.remain_quantity = calc_remain_quantity;
					return reduce_ask_quantity(odr.price, diff);
				}
				else
				{
					return book_item();
				}
				if (0 == it->second.remain_quantity)
				{
					orders.erase(it);
				}
			}
		}
		else
		{
			return book_item();
		}
	}
}




