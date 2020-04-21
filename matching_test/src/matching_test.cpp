#include <matching/engine.hpp>
#include <iostream>

void handle_order(const matching::order& o)
{
	std::string side = "";
	if (matching::order::order_side::BUY == o.side)
	{
		side = "BUY";
	}
	else
	{
		side = "SELL";
	}
	std::cout
			<< "side:" << side
			<< ",order_id:" << o.order_id
			<< ",quantity:" << o.quantity
			<< ",price:" << o.price
			<< ",remain_quantity:" << o.remain_quantity
			<< ",last_match_price:" << o.last_match_price
			<< ",last_match_quantity:" << o.last_match_quantity
			<< ",last_matched_order_id:" << o.last_matched_order_id
			<< ",matched_id:" << o.matched_id << std::endl;
}

int main()
{
	matching::engine e(handle_order);
	matching::order o;
	o.side = matching::order::order_side::BUY;
	o.price = 100;
	o.quantity = 1000;
	o.display_quantity = 1000;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.price = 99;
	o.quantity = 1200;
	o.display_quantity = 1200;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.price = 98;
	o.quantity = 800;
	o.display_quantity = 800;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.price = 97;
	o.quantity = 3200;
	o.display_quantity = 3200;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.price = 96;
	o.quantity = 500;
	o.display_quantity = 500;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.price = 96;
	o.quantity = 800;
	o.display_quantity = 800;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.price = 95;
	o.quantity = 8000;
	o.display_quantity = 8000;
	e.handle(o);

	o.side = matching::order::order_side::SELL;
	o.price = 96;
	o.quantity = 10000;
	o.display_quantity = 10000;
	e.handle(o);


	return 0;
}




