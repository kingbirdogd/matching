#include <matching/engine.hpp>
#include <iostream>
#include <unordered_map>

std::unordered_map<unsigned long long, unsigned long long> client_to_engine_id_map;

void handle_order(const matching::order& o)
{
	std::string side = "";
	std::string status = "";
	std::string time_condition = "";
	std::string action = "";
	std::string engine_type = "";
	if (matching::order::order_side::BUY == o.side)
	{
		side = "BUY";
	}
	else if (matching::order::order_side::SELL == o.side)
	{
		side = "SELL";
	}
	else if (matching::order::order_side::BUY_STOP == o.side)
	{
		side = "BUY_STOP";
	}
	else if (matching::order::order_side::SELL_STOP == o.side)
	{
		side = "SELL_STOP";
	}
	else if (matching::order::order_side::BUY_SELL_STOP == o.side)
	{
		side = "BUY_SELL_STOP";
	}
	else
	{
		side = "SELL_BUY_STOP";
	}
	if (o.order_state == matching::order::order_status_type::OPEN)
	{
		status = "OPEN";
	}
	else if (o.order_state == matching::order::order_status_type::PARTIAL_FILL)
	{
		status = "PARTIAL_FILL";
	}
	else if (o.order_state == matching::order::order_status_type::FILLED)
	{
		status = "FILLED";
	}
	else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED)
	{
		status = "CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED";
	}
	else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MARKET_ORDER_NOTHING_MATCH)
	{
		status = "CANCELED_BY_MARKET_ORDER_NOTHING_MATCH";
	}
	else if (o.order_state == matching::order::order_status_type::CANCELED_ALL_BY_IOC)
	{
		status = "CANCELED_ALL_BY_IOC";
	}
	else if (o.order_state == matching::order::order_status_type::CANCELED_PARTIAL_BY_IOC)
	{
		status = "CANCELED_PARTIAL_BY_IOC";
	}
	else if (o.order_state == matching::order::order_status_type::CANCELED_BY_FOK)
	{
		status = "CANCELED_BY_FOK";
	}
	else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MAKER_ONLY)
	{
		status = "CANCELED_BY_MAKER_ONLY";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_CANCEL_ORDER_ID_NOT_FOUND)
	{
		status = "REJECT_CANCEL_ORDER_ID_NOT_FOUND";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_AMEND_ORDER_ID_NOT_FOUND)
	{
		status = "REJECT_AMEND_ORDER_ID_NOT_FOUND";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY)
	{
		status = "REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_BUY_STOP_TRIGGER_LESS_THAN_STOP_LIMITED)
	{
		status = "REJECT_BUY_STOP_TRIGGER_LESS_THAN_STOP_LIMITED";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_BUY_STOP_NO_BEST_ASK)
	{
		status = "REJECT_BUY_STOP_NO_BEST_ASK";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_BUY_STOP_TRIGGER_LESS_THAN_BEST_ASK)
	{
		status = "REJECT_BUY_STOP_TRIGGER_LESS_THAN_BEST_ASK";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED)
	{
		status = "REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_SELL_STOP_NO_BEST_BID)
	{
		status = "REJECT_SELL_STOP_NO_BEST_BID";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_BEST_BID)
	{
		status = "REJECT_SELL_STOP_TRIGGER_LESS_THAN_BEST_BID";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_BUY_SELL_STOP_TRIGGER_CROSS)
	{
		status = "REJECT_BUY_SELL_STOP_TRIGGER_CROSS";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_UNKNOW_ORDER_ACTION)
	{
		status = "REJECT_UNKNOW_ORDER_ACTION";
	}
	else
	{
		status = "REJECT_QUANTITY_ZERO";
	}
	if (matching::order::order_time_condition::GTC == o.time_condition)
	{
		time_condition = "GTC";
	}
	else if (matching::order::order_time_condition::IOC == o.time_condition)
	{
		time_condition = "IOC";
	}
	else if (matching::order::order_time_condition::FOK == o.time_condition)
	{
		time_condition = "FOK";
	}
	else
	{
		time_condition = "MAKER_ONLY";
	}
	if (matching::order::order_action_type::NEW == o.order_action)
	{
		action = "NEW";
	}
	else if (matching::order::order_action_type::CANCEL == o.order_action)
	{
		action = "CANCEL";
	}
	else
	{
		action = "AMEND";
	}
	if (matching::order::order_engine_type::NORMAL == o.engine_type)
	{
		engine_type = "NORMAL";
	}
	else
	{
		engine_type = "IMPLIED";
	}
	client_to_engine_id_map[o.client_order_id] = o.order_id;
	std::cout
			<< "action:" << action
			<< ",side:" << side
			<< ",time_condition:" << time_condition
			<< ",order_id:" << o.order_id
			<< ",client_order_id:" << o.client_order_id
			<< ",quantity:" << o.quantity
			<< ",price:" << o.price
			<< ",remain_quantity:" << o.remain_quantity
			<< ",last_match_price:" << o.last_match_price
			<< ",last_match_quantity:" << o.last_match_quantity
			<< ",last_matched_order_id:" << o.last_matched_order_id
			<< ",matched_id:" << o.matched_id
			<< ",engine_type:" << engine_type
			<< ",status:" << status
			<< std::endl;
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
	o.time_condition = matching::order::order_time_condition::FOK;
	o.client_order_id = 200;
	o.price = 99;
	o.quantity = 8000;
	o.display_quantity = 8000;
	e.handle(o);
	//recovery GTC
	o.time_condition = matching::order::order_time_condition::GTC;

	std::cout << "FOK recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "FOK recovery end" << std::endl;

	o.side = matching::order::order_side::SELL;
	o.time_condition = matching::order::order_time_condition::FOK;
	o.client_order_id = 201;
	o.price = 99;
	o.quantity = 10;
	o.display_quantity = 5;
	e.handle(o);
	//recovery GTC
	o.time_condition = matching::order::order_time_condition::GTC;

	std::cout << "FOK success recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "FOK success recovery end" << std::endl;


	o.side = matching::order::order_side::SELL;
	o.time_condition = matching::order::order_time_condition::IOC;
	o.client_order_id = 300;
	o.price = 99;
	o.quantity = 8000;
	o.display_quantity = 8000;
	e.handle(o);
	//recovery GTC
	o.time_condition = matching::order::order_time_condition::GTC;

	std::cout << "IOC recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "IOC recovery end" << std::endl;


	o.side = matching::order::order_side::SELL;
	o.time_condition = matching::order::order_time_condition::MAKER_ONLY;
	o.client_order_id = 400;
	o.price = 92;
	o.quantity = 8000;
	o.display_quantity = 8000;
	e.handle(o);
	//recovery GTC
	o.time_condition = matching::order::order_time_condition::GTC;

	std::cout << "MAKER_ONLY cancel recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "MAKER_ONLY cancel recovery end" << std::endl;


	o.side = matching::order::order_side::SELL;
	o.time_condition = matching::order::order_time_condition::MAKER_ONLY;
	o.client_order_id = 500;
	o.price = 101;
	o.quantity = 8000;
	o.display_quantity = 8000;
	e.handle(o);
	//recovery GTC
	o.time_condition = matching::order::order_time_condition::GTC;

	std::cout << "MAKER_ONLY place recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "MAKER_ONLY place recovery end" << std::endl;

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




