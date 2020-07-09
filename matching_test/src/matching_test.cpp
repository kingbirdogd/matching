#include <matching/engine.hpp>
#include <md/md_book.hpp>
#include <add_bid_implier.hpp>
#include <add_ask_implier.hpp>
#include <minus_bid_implier.hpp>
#include <minus_ask_implier.hpp>
#include <matching/implied_spread_in_bid.hpp>
#include <matching/implied_spread_in_ask.hpp>
#include <matching/implied_spread_a_out_bid.hpp>
#include <matching/implied_spread_a_out_ask.hpp>
#include <matching/implied_spread_b_out_bid.hpp>
#include <matching/implied_spread_b_out_ask.hpp>
#include <memory/object_pool.hpp>
#include <iostream>
#include <unordered_map>
#include <cassert>
#include <cmath>

std::unordered_map<unsigned long long, unsigned long long> client_to_engine_id_map;

void handle_md(const md::book_item& item)
{
	std::string side = "";
	if (md::book_item::book_side::bid == item.side)
	{
		side = "bid";
	}
	else if (md::book_item::book_side::ask == item.side)
	{
		side = "ask";
	}
	else
	{
		side = "none";
	}
	std::cout
	<< "book_item, size:" << side
	<< ",price:" << item.price
	<< ",quantity:" << item.quantity
	<< std::endl;
}

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
	else if (o.order_state == matching::order::order_status_type::CANCELED_BY_USER)
	{
		status = "CANCELED_BY_USER";
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
  else if (o.order_state == matching::order::order_status_type::REJECT_LIMITE_ORDER_WITH_MARKET_PRICE)
  {
    status = "REJECT_LIMITE_ORDER_WITH_MARKET_PRICE";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_ALL_BY_AUCTION)
  {
    status = "CANCELED_ALL_BY_AUCTION";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_PARTIAL_BY_AUCTION)
  {
    status = "CANCELED_PARTIAL_BY_AUCTION";
  }
	else
	{
		status = "REJECT_AUCTION_SUPPORT_BUY_SELL_ONLY";
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
  else if (matching::order::order_time_condition::MAKER_ONLY_REPRICE == o.time_condition)
  {
    time_condition = "MAKER_ONLY_REPRICE";
  }
	else
	{
		time_condition = "AUCTION";
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

void stop_test()
{
	matching::engine e(handle_order);
	matching::order o;

	o.side = matching::order::order_side::SELL;
	o.client_order_id = 1;
	o.quantity = 1000;
	o.display_quantity = 1000;
	o.price = 100;
	e.handle(o);

	o.side = matching::order::order_side::SELL;
	o.client_order_id = 2;
	o.quantity = 1000;
	o.display_quantity = 1000;
	o.price = 103;
	e.handle(o);

	o.side = matching::order::order_side::BUY_STOP;
	o.client_order_id = 3;
	o.quantity = 1000;
	o.display_quantity = 1000;
	o.buy_stop_trigger_price = 102;
	o.buy_stop_limited_price = 105;
	e.handle(o);

	std::cout << "Start try trigger" << std::endl;
	o.side = matching::order::order_side::BUY;
	o.client_order_id = 4;
	o.quantity = 1500;
	o.display_quantity = 1500;
	o.price = 108;
	e.handle(o);
}

void stop_test_by_cancel()
{
	matching::engine e(handle_order);
	matching::order o;

	o.side = matching::order::order_side::SELL;
	o.client_order_id = 1;
	o.quantity = 1000;
	o.display_quantity = 1000;
	o.price = 100;
	e.handle(o);

	o.side = matching::order::order_side::SELL;
	o.client_order_id = 2;
	o.quantity = 950;
	o.display_quantity = 950;
	o.price = 103;
	e.handle(o);

	o.side = matching::order::order_side::BUY_STOP;
	o.client_order_id = 3;
	o.quantity = 1000;
	o.display_quantity = 1000;
	o.buy_stop_trigger_price = 102;
	o.buy_stop_limited_price = 105;
	e.handle(o);

	std::cout << "Start try trigger by cancel" << std::endl;
	o.order_action = matching::order::order_action_type::CANCEL;
	o.client_order_id = 1;
	o.order_id = client_to_engine_id_map[1];
	e.handle(o);

	std::cout << "Start try trigger by oderbook come out again" << std::endl;
	o.order_action = matching::order::order_action_type::NEW;
	o.side = matching::order::order_side::SELL;
	o.client_order_id = 20;
	o.quantity = 800;
	o.display_quantity = 800;
	o.price = 104;
	e.handle(o);
}

void test_object_pool()
{
	memory::object_pool<matching::order, 1024> pool;
	auto ptr = pool.alloc();
	if (ptr)
	{
		std::cout << "alloc success" << std::endl;
		handle_order(*ptr);
		if (pool.free(ptr))
		{
			std::cout << "free success" << std::endl;
		}
	}
};

void implied_test_md_tick_size()
{                                   //905050000000
  unsigned long long  march_tick_size  =  50000000;
  unsigned long long  june_tick_size   =  50000000;
  unsigned long long  spread_tick_size = 100000000;
                                   // 905199990000
                                   // 905100000000
  add_bid_implier abi(0);
  add_ask_implier aai(0);
  minus_bid_implier mbi(0);
  minus_ask_implier mai(0);

  //SHORT A(implied OUT, BID_A) <- (BID_PRICE_AB + BID_PRICE_B) @ MIN(BID_QUANTITY_AB, BID_QUANTITY_B)
  //LONG A(implied OUT, ASK_A) <- (ASK_PRICE_AB + ASK_PRICE_B) @ MIN(ASK_QUANTITY_AB, ASK_QUANTITY_B)
  md::md_book March_Book
      (
          //handle_md,
          [](const md::book_item& item){},
          march_tick_size,
          md::md_book::implier_type::a_bid_b_bid,
          md::md_book::implier_type::a_ask_b_ask,
          &abi,
          &aai
      );

  //SHORT B(implied OUT, BID_B) <- (BID_PRICE_A - ASK_PRICE_AB) @ MIN(BID_QUANTITY_A, ASK_QUANTITY_AB)
  //LONG B(implied OUT, ASK_B) <- (ASK_PRICE_A - BID_PRICE_AB) @ MIN(ASK_QUANTITY_A, BID_QUANTITY_AB)
  md::md_book June_Book
      (
          handle_md,
          june_tick_size,
          md::md_book::implier_type::a_bid_b_ask,
          md::md_book::implier_type::a_ask_b_bid,
          &mbi,
          &mai
      );

  //SHORT AB (implied IN, BID_AB) <- (BID_PRICE_A - ASK_PRICE_B) @ MIN(BID_QUANTITY_A, ASK_QUANTITY_B)
  //LONG AB (implied IN, ASK_AB) <- (ASK_PRICE_A - BID_PRICE_B) @ MIN(ASK_QUANTITY_A, BID_QUANTITY_B)
  md::md_book Spread_Book
      (
          //handle_md,
          [](const md::book_item& item){},
          spread_tick_size,
          md::md_book::implier_type::a_bid_b_ask,
          md::md_book::implier_type::a_ask_b_bid,
          &mbi,
          &mai
      );

  matching::engine March
      (
          [&](const matching::order& odr)
          {
            handle_order(odr);
            March_Book.handle_outright(odr);
            June_Book.handle_a(odr);
            Spread_Book.handle_a(odr);
          },
          march_tick_size
      );
  matching::engine June
      (
          [&](const matching::order& odr)
          {
            handle_order(odr);
            June_Book.handle_outright(odr);
            March_Book.handle_b(odr);
            Spread_Book.handle_b(odr);
          },
          june_tick_size
      );
  matching::engine Spread
      (
          [&](const matching::order& odr)
          {
            handle_order(odr);
            Spread_Book.handle_outright(odr);
            March_Book.handle_a(odr);
            June_Book.handle_b(odr);
          },
          spread_tick_size
      );
  matching::implied_spread_in_bid spread_bid_implier(1, &March, &June);
  matching::implied_spread_in_ask spread_ask_implier(1, &March, &June);
  matching::implied_spread_a_out_bid a_bid_implier(1, &Spread, &June);
  matching::implied_spread_a_out_ask a_ask_implier(1, &Spread, &June);
  matching::implied_spread_b_out_bid b_bid_implier(1, &March, &Spread);
  matching::implied_spread_b_out_ask b_ask_implier(1, &March, &Spread);
  Spread.set_bid_implier(&spread_bid_implier);
  Spread.set_ask_implier(&spread_ask_implier);
  March.set_bid_implier(&a_bid_implier);
  March.set_ask_implier(&a_ask_implier);
  June.set_bid_implier(&b_bid_implier);
  June.set_ask_implier(&b_ask_implier);
  matching::order o;

  o.side = matching::order::order_side::BUY;   // Maker remain qty overflow
  o.client_order_id = 2;
  o.price            =   965355117777;
                       //   777777770;
  //o.price          =   905100000000;
  o.quantity         =   200;
  o.display_quantity =   200;
  March.handle(o);

  o.side = matching::order::order_side::SELL;   // Maker remain qty overflow
  o.client_order_id = 4;
  o.price            = 777777770;
  o.quantity         =   400;
  o.display_quantity =   400;
  //o.time_condition   = matching::order::MAKER_ONLY_REPRICE;
  Spread.handle(o);

  std::cout << std::endl;

  o.side = matching::order::order_side::BUY;   // Maker remain qty overflow
  o.client_order_id = 3;
  o.price            =   967199999999;
  o.quantity         =   300;
  o.display_quantity =   300;
  o.time_condition   = matching::order::MAKER_ONLY_REPRICE;
  June.handle(o);

  o.side = matching::order::order_side::SELL;   // Maker remain qty overflow
  o.client_order_id = 5;
  o.price            =   960100000000;
  o.quantity         =   300;
  o.display_quantity =   300;
  o.time_condition   = matching::order::MAKER_ONLY_REPRICE;
  June.handle(o);

}

void implied_test_remain_qty_overflow()
{
  add_bid_implier abi(0);
  add_ask_implier aai(0);
  minus_bid_implier mbi(0);
  minus_ask_implier mai(0);

  //SHORT A(implied OUT, BID_A) <- (BID_PRICE_AB + BID_PRICE_B) @ MIN(BID_QUANTITY_AB, BID_QUANTITY_B)
  //LONG A(implied OUT, ASK_A) <- (ASK_PRICE_AB + ASK_PRICE_B) @ MIN(ASK_QUANTITY_AB, ASK_QUANTITY_B)
  md::md_book March_Book
      (
          handle_md,
          1,
          md::md_book::implier_type::a_bid_b_bid,
          md::md_book::implier_type::a_ask_b_ask,
          &abi,
          &aai
      );

  //SHORT B(implied OUT, BID_B) <- (BID_PRICE_A - ASK_PRICE_AB) @ MIN(BID_QUANTITY_A, ASK_QUANTITY_AB)
  //LONG B(implied OUT, ASK_B) <- (ASK_PRICE_A - BID_PRICE_AB) @ MIN(ASK_QUANTITY_A, BID_QUANTITY_AB)
  md::md_book June_Book
      (
          handle_md,
          1,
          md::md_book::implier_type::a_bid_b_ask,
          md::md_book::implier_type::a_ask_b_bid,
          &mbi,
          &mai
      );

  //SHORT AB (implied IN, BID_AB) <- (BID_PRICE_A - ASK_PRICE_B) @ MIN(BID_QUANTITY_A, ASK_QUANTITY_B)
  //LONG AB (implied IN, ASK_AB) <- (ASK_PRICE_A - BID_PRICE_B) @ MIN(ASK_QUANTITY_A, BID_QUANTITY_B)
  md::md_book Spread_Book
      (
          handle_md,
          1,
          md::md_book::implier_type::a_bid_b_ask,
          md::md_book::implier_type::a_ask_b_bid,
          &mbi,
          &mai
      );

  matching::engine March
      (
          [&](const matching::order& odr)
          {
            handle_order(odr);
            March_Book.handle_outright(odr);
            June_Book.handle_a(odr);
            Spread_Book.handle_a(odr);
          }
      );
  matching::engine June
      (
          [&](const matching::order& odr)
          {
            handle_order(odr);
            June_Book.handle_outright(odr);
            March_Book.handle_b(odr);
            Spread_Book.handle_b(odr);

          }
      );
  matching::engine Spread
      (
          [&](const matching::order& odr)
          {
            handle_order(odr);
            Spread_Book.handle_outright(odr);
            March_Book.handle_a(odr);
            June_Book.handle_b(odr);
          }
      );
  matching::implied_spread_in_bid spread_bid_implier(1, &March, &June);
  matching::implied_spread_in_ask spread_ask_implier(1, &March, &June);
  matching::implied_spread_a_out_bid a_bid_implier(1, &Spread, &June);
  matching::implied_spread_a_out_ask a_ask_implier(1, &Spread, &June);
  matching::implied_spread_b_out_bid b_bid_implier(1, &March, &Spread);
  matching::implied_spread_b_out_ask b_ask_implier(1, &March, &Spread);
  Spread.set_bid_implier(&spread_bid_implier);
  Spread.set_ask_implier(&spread_ask_implier);
  March.set_bid_implier(&a_bid_implier);
  March.set_ask_implier(&a_ask_implier);
  June.set_bid_implier(&b_bid_implier);
  June.set_ask_implier(&b_ask_implier);
  matching::order o;

  o.side = matching::order::order_side::SELL;  // Maker
  o.client_order_id = 276;
  o.price            = 972000000000;
  o.quantity         =   3000000000;
  o.display_quantity =   3000000000;
  March.handle(o);

  o.side = matching::order::order_side::SELL;  // Maker remain qty overflow
  o.client_order_id = 277;
  o.price            = 972000000000;
  o.quantity         =  10000000000;
  o.display_quantity =  10000000000;
  March.handle(o);

  o.side = matching::order::order_side::BUY;   // Maker remain qty overflow
  o.client_order_id = 394;
  o.price            =  889800000000;
  o.quantity         =   10900000000;
  o.display_quantity =   10900000000;
  June.handle(o);

  o.side = matching::order::order_side::BUY;  // Taker
  o.client_order_id = 454;
  o.price            = 1000000000000;
  o.quantity         =    1200000000;
  o.display_quantity =    1200000000;
  Spread.handle(o);

  o.side = matching::order::order_side::BUY;  // Taker
  o.client_order_id = 473;
  o.price            =  100000000000;
  o.quantity         =  100000000000;
  o.display_quantity =  100000000000;
  Spread.handle(o);
}

void implied_test()
{
	add_bid_implier abi(0);
	add_ask_implier aai(0);
	minus_bid_implier mbi(0);
	minus_ask_implier mai(0);

	//SHORT A(implied OUT, BID_A) <- (BID_PRICE_AB + BID_PRICE_B) @ MIN(BID_QUANTITY_AB, BID_QUANTITY_B)
	//LONG A(implied OUT, ASK_A) <- (ASK_PRICE_AB + ASK_PRICE_B) @ MIN(ASK_QUANTITY_AB, ASK_QUANTITY_B)
	md::md_book March_Book
	(
			handle_md,
			1,
			md::md_book::implier_type::a_bid_b_bid,
			md::md_book::implier_type::a_ask_b_ask,
			&abi,
			&aai
	);

	//SHORT B(implied OUT, BID_B) <- (BID_PRICE_A - ASK_PRICE_AB) @ MIN(BID_QUANTITY_A, ASK_QUANTITY_AB)
	//LONG B(implied OUT, ASK_B) <- (ASK_PRICE_A - BID_PRICE_AB) @ MIN(ASK_QUANTITY_A, BID_QUANTITY_AB)
	md::md_book June_Book
	(
			handle_md,
			1,
			md::md_book::implier_type::a_bid_b_ask,
			md::md_book::implier_type::a_ask_b_bid,
			&mbi,
			&mai
	);

	//SHORT AB (implied IN, BID_AB) <- (BID_PRICE_A - ASK_PRICE_B) @ MIN(BID_QUANTITY_A, ASK_QUANTITY_B)
	//LONG AB (implied IN, ASK_AB) <- (ASK_PRICE_A - BID_PRICE_B) @ MIN(ASK_QUANTITY_A, BID_QUANTITY_B)
	md::md_book Spread_Book
	(
			handle_md,
			1,
			md::md_book::implier_type::a_bid_b_ask,
			md::md_book::implier_type::a_ask_b_bid,
			&mbi,
			&mai
	);

	matching::engine March
	(
		[&](const matching::order& odr)
		{
			handle_order(odr);
			March_Book.handle_outright(odr);
			June_Book.handle_a(odr);
			Spread_Book.handle_a(odr);
		}
	);
	matching::engine June
	(
		[&](const matching::order& odr)
		{
			handle_order(odr);
			June_Book.handle_outright(odr);
			March_Book.handle_b(odr);
			Spread_Book.handle_b(odr);

		}
	);
	matching::engine Spread
	(
		[&](const matching::order& odr)
		{
			handle_order(odr);
			Spread_Book.handle_outright(odr);
			March_Book.handle_a(odr);
			June_Book.handle_b(odr);
		}
	);
	matching::implied_spread_in_bid spread_bid_implier(1, &March, &June);
	matching::implied_spread_in_ask spread_ask_implier(1, &March, &June);
	matching::implied_spread_a_out_bid a_bid_implier(1, &Spread, &June);
	matching::implied_spread_a_out_ask a_ask_implier(1, &Spread, &June);
	matching::implied_spread_b_out_bid b_bid_implier(1, &March, &Spread);
	matching::implied_spread_b_out_ask b_ask_implier(1, &March, &Spread);
	Spread.set_bid_implier(&spread_bid_implier);
	Spread.set_ask_implier(&spread_ask_implier);
	March.set_bid_implier(&a_bid_implier);
	March.set_ask_implier(&a_ask_implier);
	June.set_bid_implier(&b_bid_implier);
	June.set_ask_implier(&b_ask_implier);
	matching::order o;

	o.side = matching::order::order_side::SELL;
	o.client_order_id = 1;
	o.price = 9500;
	o.quantity = 100;
	o.display_quantity = 100;
	March.handle(o);

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 2;
	o.price = 9450;
	o.quantity = 50;
	o.display_quantity = 50;
	June.handle(o);

	o.side = matching::order::order_side::SELL;
	o.client_order_id = 3;
	o.price = 50;
	o.quantity = 30;
	o.display_quantity = 30;
	Spread.handle(o);

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 4;
	o.price = 50;
	o.quantity = 100;
	o.display_quantity = 100;
	Spread.handle(o);
}

void implied_test_case_5()
{
  matching::engine March(handle_order);
  matching::engine June(handle_order);
  matching::engine Spread(handle_order);
  matching::implied_spread_in_bid spread_bid_implier(1, &March, &June);
  matching::implied_spread_in_ask spread_ask_implier(1, &March, &June);
  matching::implied_spread_a_out_bid a_bid_implier(1, &Spread, &June);
  matching::implied_spread_a_out_ask a_ask_implier(1, &Spread, &June);
  matching::implied_spread_b_out_bid b_bid_implier(1, &March, &Spread);
  matching::implied_spread_b_out_ask b_ask_implier(1, &March, &Spread);
  Spread.set_bid_implier(&spread_bid_implier);
  Spread.set_ask_implier(&spread_ask_implier);
  March.set_bid_implier(&a_bid_implier);
  March.set_ask_implier(&a_ask_implier);
  June.set_bid_implier(&b_bid_implier);
  June.set_ask_implier(&b_ask_implier);

  matching::order o;

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 1;
  o.price = 20;
  o.quantity = 5;
  o.display_quantity = 5;
  June.handle(o);

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 2;
  o.price = 10;
  o.quantity = 5;
  o.display_quantity = 5;
  Spread.handle(o);

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 3;
  o.price = 30;
  o.quantity = 5;
  o.display_quantity = 5;
  March.handle(o);
}

void test_case_1()
{
	matching::engine e(handle_order);
	matching::order o;


	o.side = matching::order::order_side::SELL;
	o.client_order_id = 30010;
	o.price = 100;
	o.quantity = 2000;
	o.display_quantity = 2000;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 30011;
	o.price = 99;
	o.quantity = 2000;
	o.display_quantity = 2000;
	e.handle(o);

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 30012;
	o.price = 98;
	o.quantity = 2000;
	o.display_quantity = 2000;
	e.handle(o);

	o.side = matching::order::order_side::SELL_STOP;
	o.client_order_id = 30013;
	o.sell_stop_trigger_price = 98;
	o.sell_stop_limited_price = 96;
	o.quantity = 300;
	o.display_quantity = 300;
	e.handle(o);

	o.side = matching::order::order_side::SELL;
	o.client_order_id = 30014;
	o.price = 98;
	o.quantity = 2500;
	o.display_quantity = 2500;
	e.handle(o);
	return;
}

void test_case_2()
{
  matching::engine e(handle_order);
  matching::order o;


  o.side = matching::order::order_side::SELL;
  o.client_order_id = 30020;
  o.price = 100;
  o.quantity = 2000;
  o.display_quantity = 2000;
  e.handle(o);

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 30021;
  o.price = 98;
  o.quantity = 2000;
  o.display_quantity = 2000;
  e.handle(o);

  o.side = matching::order::order_side::SELL_STOP;
  o.client_order_id = 30022;
  o.sell_stop_trigger_price = 98;
  o.sell_stop_limited_price = 96;
  o.quantity = 300;
  o.display_quantity = 300;
  e.handle(o);

  return;
}

void auction_test_auction_buy1() {
  printf("====== %s ======\n", __FUNCTION__);
  unsigned long long factor = 100000000;
  matching::order output_order;
  matching::engine e([&](const matching::order& o) { handle_order(o);output_order = o; }, factor);
  matching::order o;

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 30020;
  o.price = std::round(0.0001 * factor);
  o.quantity = 6;
  o.display_quantity = 6;
  e.handle(o);

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 11111;
  o.price =  std::round(98 * factor);
  o.quantity = 2000;
  o.display_quantity = 2000;
  o.time_condition = matching::order::AUCTION;
  o.buy_stop_limited_price = std::round(0.01 * factor);
  e.handle(o);

  assert(output_order.remain_quantity   == 1994);
  assert(output_order.last_match_price  == std::round(0.01 * factor));

  return;
}

void auction_test_auction_buy2() {
  printf("====== %s ======\n", __FUNCTION__);
  unsigned long long factor = 100000000;
  matching::order output_order;
  matching::engine e([&](const matching::order& o) { handle_order(o);output_order = o; }, factor);
  matching::order o;

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 30020;
  o.price =  std::round(0.0001 * factor);
  o.quantity = 6;
  o.display_quantity = 6;
  e.handle(o);

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 30021;
  o.price =  std::round(0.00015 * factor);
  o.quantity = 7;
  o.display_quantity = 7;
  e.handle(o);

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 11111;
  o.price = 98 * factor;
  o.quantity = 2000;
  o.display_quantity = 2000;
  o.time_condition = matching::order::AUCTION;
  o.buy_stop_limited_price = std::round(0.01 * factor);
  e.handle(o);

  assert(output_order.remain_quantity   == 1987);
  assert(output_order.last_match_price  == std::round(0.01 * factor));

  return;
}

void auction_test_auction_buy3() {
  printf("====== %s ======\n", __FUNCTION__);
  unsigned long long factor = 100000000;
  matching::order output_order;
  matching::engine e([&](const matching::order& o) { handle_order(o);output_order = o; }, factor);
  matching::order o;

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 30020;
  o.price =  std::round(-0.0001 * factor);
  o.quantity = 6;
  o.display_quantity = 6;
  e.handle(o);

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 30021;
  o.price =  std::round(-0.00015 * factor);
  o.quantity = 7;
  o.display_quantity = 7;
  e.handle(o);

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 11111;
  o.price =  std::round(98 * factor);
  o.quantity = 2000;
  o.display_quantity = 2000;
  o.time_condition = matching::order::AUCTION;
  o.buy_stop_limited_price = std::round(0.01 * factor);
  e.handle(o);

  assert(output_order.remain_quantity   == 1987);
  assert(output_order.last_match_price  == std::round(0.01 * factor));

  return;
}

void auction_test_auction_buy4() {   // FULL FILLED, last match price set to final matched price level
  printf("====== %s ======\n", __FUNCTION__);
  unsigned long long factor = 100000000;
  matching::order output_order;
  matching::engine e([&](const matching::order& o) { handle_order(o);output_order = o; }, factor);
  matching::order o;

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 30020;
  o.price =  std::round(0.0001 * factor);
  o.quantity = 600;
  o.display_quantity = 600;
  e.handle(o);

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 30021;
  o.price =  std::round(0.00015 * factor);
  o.quantity = 700;
  o.display_quantity = 700;
  e.handle(o);

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 11111;
  o.price =  std::round(98 * factor);
  o.quantity = 1000;
  o.display_quantity = 1000;
  o.time_condition = matching::order::AUCTION;
  o.buy_stop_limited_price = std::round(0.01 * factor);
  e.handle(o);

  assert(output_order.remain_quantity   == 0);
  assert(output_order.last_match_price  == std::round(0.00015 * factor));

  return;
}

void auction_test_auction_buy5() {   // FULL FILLED, last match price set to 0
  printf("====== %s ======\n", __FUNCTION__);
  unsigned long long factor = 100000000;
  matching::order output_order;
  matching::engine e([&](const matching::order& o) { handle_order(o);output_order = o; }, factor);
  matching::order o;

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 30020;
  o.price =  std::round(-0.0001 * factor);
  o.quantity = 600;
  o.display_quantity = 600;
  e.handle(o);

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 30021;
  o.price =  std::round(-0.00015 * factor);
  o.quantity = 700;
  o.display_quantity = 700;
  e.handle(o);

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 11111;
  o.price =  std::round(98 * factor);
  o.quantity = 1000;
  o.display_quantity = 1000;
  o.time_condition = matching::order::AUCTION;
  o.buy_stop_limited_price = std::round(0.01 * factor);
  e.handle(o);

  assert(output_order.remain_quantity   == 0);
  assert(output_order.last_match_price  == std::round(0 * factor));

  return;
}

void auction_test_auction_sell1() {
  printf("====== %s ======\n", __FUNCTION__);
  unsigned long long factor = 100000000;
  matching::order output_order;
  matching::engine e([&](const matching::order& o) { handle_order(o);output_order = o; }, factor);
  matching::order o;

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 30020;
  o.price = std::round(-0.0001 * factor);
  o.quantity = 6;
  o.display_quantity = 6;
  e.handle(o);

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 11111;
  o.price =  std::round(-98 * factor);
  o.quantity = 2000;
  o.display_quantity = 2000;
  o.time_condition = matching::order::AUCTION;
  o.sell_stop_limited_price = std::round(-0.01 * factor);
  e.handle(o);

  assert(output_order.remain_quantity   == 1994);
  assert(output_order.last_match_price  == std::round(-0.01 * factor));


  return;
}

void auction_test_auction_sell2() {
  printf("====== %s ======\n", __FUNCTION__);
  unsigned long long factor = 100000000;
  matching::order output_order;
  matching::engine e([&](const matching::order& o) { handle_order(o);output_order = o; }, factor);
  matching::order o;

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 30020;
  o.price =  std::round(-0.0001 * factor);
  o.quantity = 6;
  o.display_quantity = 6;
  e.handle(o);

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 30021;
  o.price =  std::round(-0.00015 * factor);
  o.quantity = 7;
  o.display_quantity = 7;
  e.handle(o);

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 11111;
  o.price = -98 * factor;
  o.quantity = 2000;
  o.display_quantity = 2000;
  o.time_condition = matching::order::AUCTION;
  o.sell_stop_limited_price = std::round(-0.01 * factor);
  e.handle(o);

  assert(output_order.remain_quantity   == 1987);
  assert(output_order.last_match_price  == std::round(-0.01 * factor));

  return;
}

void auction_test_auction_sell3() {
  printf("====== %s ======\n", __FUNCTION__);
  unsigned long long factor = 100000000;
  matching::order output_order;
  matching::engine e([&](const matching::order& o) { handle_order(o);output_order = o; }, factor);
  matching::order o;

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 30020;
  o.price =  std::round(0.0001 * factor);
  o.quantity = 6;
  o.display_quantity = 6;
  e.handle(o);

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 30021;
  o.price =  std::round(0.00015 * factor);
  o.quantity = 7;
  o.display_quantity = 7;
  e.handle(o);

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 11111;
  o.price =  std::round(-98 * factor);
  o.quantity = 2000;
  o.display_quantity = 2000;
  o.time_condition = matching::order::AUCTION;
  o.sell_stop_limited_price = std::round(-0.01 * factor);
  e.handle(o);

  assert(output_order.remain_quantity   == 1987);
  assert(output_order.last_match_price  == std::round(-0.01 * factor));

  return;
}

void auction_test_auction_sell4() {   // FULL FILLED, last match price set to final matched price level
  printf("====== %s ======\n", __FUNCTION__);
  unsigned long long factor = 100000000;
  matching::order output_order;
  matching::engine e([&](const matching::order& o) { handle_order(o);output_order = o; }, factor);
  matching::order o;

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 30020;
  o.price =  std::round(-0.0001 * factor);
  o.quantity = 600;
  o.display_quantity = 600;
  e.handle(o);

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 30021;
  o.price =  std::round(-0.00015 * factor);
  o.quantity = 700;
  o.display_quantity = 700;
  e.handle(o);

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 11111;
  o.price =  std::round(-98 * factor);
  o.quantity = 1000;
  o.display_quantity = 1000;
  o.time_condition = matching::order::AUCTION;
  o.sell_stop_limited_price = std::round(-0.01 * factor);
  e.handle(o);

  assert(output_order.remain_quantity   == 0);
  assert(output_order.last_match_price  == std::round(-0.00015 * factor));

  return;
}

void auction_test_auction_sell5() {   // FULL FILLED, last match price set to 0
  printf("====== %s ======\n", __FUNCTION__);
  unsigned long long factor = 100000000;
  matching::order output_order;
  matching::engine e([&](const matching::order& o) { handle_order(o);output_order = o; }, factor);
  matching::order o;

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 30020;
  o.price =  std::round(0.0001 * factor);
  o.quantity = 600;
  o.display_quantity = 600;
  e.handle(o);

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 30021;
  o.price =  std::round(0.00015 * factor);
  o.quantity = 700;
  o.display_quantity = 700;
  e.handle(o);

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 11111;
  o.price =  std::round(-98 * factor);
  o.quantity = 1000;
  o.display_quantity = 1000;
  o.time_condition = matching::order::AUCTION;
  o.sell_stop_limited_price = std::round(0.01 * factor);
  e.handle(o);

  assert(output_order.remain_quantity   == 0);
  assert(output_order.last_match_price  == std::round(0 * factor));

  return;
}

void test_reprice1() {
  printf("====== %s ======\n", __FUNCTION__);
  unsigned long long factor  = 100000000;
  double tick_sz = 0.1;
  matching::order output_order;
  matching::engine e([&](const matching::order& o) { handle_order(o);output_order = o; }, factor*tick_sz);
  matching::order o;

  o.side = matching::order::order_side::BUY;
  o.client_order_id = 1001;
  o.price = 950000000000; //  std::round(0.0001 * factor);
  o.quantity = 5000000000;
  o.display_quantity = 5000000000;
  e.handle(o);

  o.side = matching::order::order_side::SELL;
  o.client_order_id = 1002;
  o.price = 915500000000; //  std::round(0.0001 * factor);
  o.quantity = 2000000000;
  o.display_quantity = 2000000000;
  o.time_condition = matching::order::MAKER_ONLY_REPRICE;
  e.handle(o);

}

int main()
{
  matching::engine e(handle_order);
  matching::order o;
  implied_test_md_tick_size();
  //test_reprice1();
  /*
//  auction_test_auction_buy1();
//  auction_test_auction_buy2();
//  auction_test_auction_buy3();
//  auction_test_auction_buy4();
//  auction_test_auction_buy5();
//  auction_test_auction_sell1();
//  auction_test_auction_sell2();
//  auction_test_auction_sell3();
//  auction_test_auction_sell4();
//  auction_test_auction_sell5();

  //implied_test_md_tick_size();
  //implied_test_remain_qty_overflow();
	//implied_test();
	//implied_test_case_5();
  test_case_2();
	test_case_1();
	implied_test();
	stop_test();
	stop_test_by_cancel();

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 1;
	o.price = 100;
	o.quantity = 6000;
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
	o.time_condition = matching::order::order_time_condition::MAKER_ONLY_REPRICE;
	o.client_order_id = 500;
	o.price = 90;
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
//	o.order_action = matching::order::order_action_type::CANCEL;
//	o.client_order_id = 7;
//	o.order_id = client_to_engine_id_map[7];
//  o.client_order_id = 0;
//	e.handle(o);
	std::cout << "Thrid recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "Thrid recovery end" << std::endl;

	o.order_action = matching::order::order_action_type::AMEND;
	o.client_order_id = 0;
	o.order_id = client_to_engine_id_map[8];
	o.quantity = 15000;
	o.display_quantity = 15000;
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
	o.side = matching::order::order_side::BUY;
	o.client_order_id = 8;
	o.order_id = client_to_engine_id_map[8];
	o.quantity = 200;
	o.display_quantity = 200;
	e.handle(o);
	std::cout << "6th recovery start" << std::endl;
	e.recovery(handle_order);
	std::cout << "6th recovery end" << std::endl;

	test_object_pool();
*/
	return 0;
}




