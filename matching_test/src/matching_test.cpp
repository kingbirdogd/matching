#include <matching/engine.hpp>
#include <iostream>
#include <unordered_map>

std::unordered_map<unsigned long long, unsigned long long> client_to_engine_id_map;

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
	client_to_engine_id_map[o.client_order_id] = o.order_id;
	std::cout
			<< "side:" << side
			<< ",order_id:" << o.order_id
			<< ",client_order_id:" << o.client_order_id
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
	o.client_order_id = 1;
	o.price = 100;
	o.quantity = 1000;
	o.display_quantity = 1000;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 2;
	o.price = 99;
	o.quantity = 1200;
	o.display_quantity = 1200;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 3;
	o.price = 98;
	o.quantity = 800;
	o.display_quantity = 800;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 4;
	o.price = 97;
	o.quantity = 3200;
	o.display_quantity = 3200;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 5;
	o.price = 96;
	o.quantity = 500;
	o.display_quantity = 500;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 6;
	o.price = 96;
	o.quantity = 800;
	o.display_quantity = 800;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 7;
	o.price = 95;
	o.quantity = 8000;
	o.display_quantity = 8000;
	e.handle(o);

	std::cout << "First recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "First recovery end" << std::endl;

	o.side = matching::order::order_side::SELL;
	o.client_order_id = 8;
	o.price = 96;
	o.quantity = 10000;
	o.display_quantity = 10000;
	e.handle(o);

	std::cout << "Second recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "Second recovery end" << std::endl;

	o.order_action = matching::order::order_action_type::CANCEL;
	o.client_order_id = 7;
	o.order_id = client_to_engine_id_map[7];
	e.handle(o);

	std::cout << "Thrid recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "Thrid recovery end" << std::endl;


	o.order_action = matching::order::order_action_type::AMEND;
	o.client_order_id = 8;
	o.order_id = client_to_engine_id_map[8];
	o.quantity = 1500;
	o.display_quantity = 1500;
	e.handle(o);

	std::cout << "4th recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "4th recovery end" << std::endl;

	o.order_action = matching::order::order_action_type::AMEND;
	o.client_order_id = 8;
	o.order_id = client_to_engine_id_map[8];
	o.quantity = 1800;
	o.display_quantity = 1800;
	e.handle(o);

	std::cout << "5th recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "5th recovery end" << std::endl;

	o.order_action = matching::order::order_action_type::AMEND;
	o.side = o.side = matching::order::order_side::BUY;
	o.client_order_id = 8;
	o.order_id = client_to_engine_id_map[8];
	o.quantity = 200;
	o.display_quantity = 200;
	e.handle(o);

	std::cout << "6th recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "6th recovery end" << std::endl;

	return 0;
}




