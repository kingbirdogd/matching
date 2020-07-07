#include <matching_tcp_client.hpp>
#include <iostream>
#include <unordered_map>
#include <string>
#include <thread>
#include <unistd.h>

std::unordered_map<unsigned long long, unsigned long long> client_to_engine_id_map;

void handle_order(const matching::order& o)
{
	std::string type = "";
	std::string side = "";
	std::string status = "";
	std::string time_condition = "";
	std::string action = "";
	std::string matched_type = "";
	if (matching::order::order_type::LIMITED == o.type)
	{
		type = "LIMITED";
	}
	else
	{
		type = "MARKET";
	}
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
	else if (o.order_state == matching::order::order_status_type::CANCELED_BY_USER)
	{
		status = "CANCELED_BY_USER";
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
	else if (o.order_state == matching::order::order_status_type::REJECT_BUY_STOP_TRIGGER_LARGE_THAN_STOP_LIMITED)
	{
		status = "REJECT_BUY_STOP_TRIGGER_LESS_THAN_STOP_LIMITED";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED)
	{
		status = "REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_UNKNOW_ORDER_ACTION)
	{
		status = "REJECT_UNKNOW_ORDER_ACTION";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_QUANTITY_ZERO)
	{
		status = "REJECT_QUANTITY_ZERO";
	}
	else
	{
		status = "REJECT_LIMITE_ORDER_WITH_MARKET_PRICE";
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
	else if (matching::order::order_time_condition::MAKER_ONLY == o.time_condition)
	{
		time_condition = "MAKER_ONLY";
	}
	else
	{
		time_condition = "MAKER_ONLY_REPRICE";
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
	if (matching::order::order_matched_type::TAKER == o.matched_type)
	{
		matched_type = "TAKER";
	}
	else
	{
		matched_type = "MAKER";
	}
	client_to_engine_id_map[o.client_order_id] = o.order_id;
	std::cout
			<< "action:" << action
			<< ",side:" << side
			<< ",time_condition:" << time_condition
			<< ",order_id:" << o.order_id
			<< ",client_order_id:" << o.client_order_id
			<< ",quantity:" << o.quantity
			<< ",display_quantity:" << o.display_quantity
			<< ",remain_quantity:" << o.remain_quantity
			<< ",price:" << o.price
			<< ",buy_stop_trigger_price:" << o.buy_stop_trigger_price
			<< ",buy_stop_limited_price:" << o.buy_stop_limited_price
			<< ",sell_stop_trigger_price:" << o.sell_stop_trigger_price
			<< ",sell_stop_limited_price:" << o.sell_stop_limited_price
			<< ",last_match_price:" << o.last_match_price
			<< ",last_match_quantity:" << o.last_match_quantity
			<< ",last_matched_order_id:" << o.last_matched_order_id
			<< ",last_matched_order_id2:" << o.last_matched_order_id2
			<< ",matched_id:" << o.matched_id
			<< ",status:" << status
			<< ",matched_type:" << matched_type
			<< std::endl;
}

int main(int iArgc, char** pszArgv)
{
	if (12 != iArgc)
	{
		std::cout << "usage: test_tcp_matching_client host <port[1,65535]> qty_factor px_factor account_id market_id qty px [BUY|SELL] buy_upper_band sell_lower_band" << std::endl;
		return -1;
	}
	std::string host = pszArgv[1];
	int iPort = std::atoi(pszArgv[2]);
	if (iPort < 1 || iPort > 65535)
	{
		std::cout << "usage: test_tcp_matching_client host <port[1,65535]>" << std::endl;
		return -2;
	}
	auto sPort = static_cast<unsigned short int>(iPort);
  unsigned long long qty_factor = strtoull(pszArgv[3], nullptr, 10);
  unsigned long long px_factor  = strtoull(pszArgv[4], nullptr, 10);
	unsigned long long account_id = strtoull(pszArgv[5], nullptr, 10);
  unsigned long long market_id  = strtoull(pszArgv[6], nullptr, 10);
  double             qty        = strtod(  pszArgv[7], nullptr);
  double             px         = strtod(  pszArgv[8], nullptr);
  matching::order::order_side side = (strcmp(pszArgv[9], "SELL") == 0) ? matching::order::order_side::SELL : matching::order::order_side::BUY;
  long long buy_upper_band  = strtod(pszArgv[10], nullptr);
  long long sell_lower_band = strtod(pszArgv[11], nullptr);

  matching_tcp_client c(host, sPort);
	c.set_connected([&]()
	{
		std::cout << "matching_tcp_client connected" << std::endl;
	});
	c.set_disconnected([&]()
	{
		std::cout << "matching_tcp_client disconnected" << std::endl;
	});
	c.set_on_order([&](const matching::order& o)
	{
		//handle_order(o);
	});

	bool done = false;
	std::thread th([&]()
	{
		while (!done)
		{
			c.run();
		}
	});

	matching::order o;
	o.account_id       = account_id;
	o.market_id        = market_id;
	o.side             = side;
	o.quantity         = qty_factor*qty;
	o.display_quantity = qty_factor*qty;
	o.price            = px_factor*px;
  o.client_order_id  = 1;
  o.time_condition   = matching::order::order_time_condition::AUCTION;

  // Use xxx_stop_limited_price to set lower/upper price band for auction
  if (o.side == matching::order::BUY)
    o.buy_stop_limited_price = px_factor*buy_upper_band;
  else if (o.side == matching::order::SELL)
    o.sell_stop_limited_price = px_factor*sell_lower_band;

	c.send(o);

	sleep(3);
	done = true;
	th.join();
	return 0;
}




