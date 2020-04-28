#include <matching/engine.hpp>
#include <ctime>

using namespace matching;

std::atomic<unsigned long long> engine::_id(static_cast<unsigned long long>(time(nullptr)) * 1000000000);

engine::engine(callback_type&& callback, long long _mini_tick):
		_odr_map(),
		_bid_book(),
		_ask_book(),
		_bid_stop_book(),
		_ask_stop_book(),
		_callback(std::move(callback)),
		_mini_tick(_mini_tick)
{
}

engine::~engine()
{
}

void engine::_handle(implied_order& o)
{
	if (order::order_action_type::NEW == o.order_action)
	{
		o.order_state = order::order_status_type::OPEN;
		auto before_best_bid = get_best_price(_bid_book);
		auto before_best_ask = get_best_price(_ask_book);
		if (handle_new(o))
		{
			auto after_best_bid = get_best_price(_bid_book);
			auto after_best_ask = get_best_price(_ask_book);
			handle_stop(before_best_bid,
					before_best_ask,
					after_best_bid,
					after_best_ask);
		}
	}
	else if (order::order_action_type::CANCEL == o.order_action)
	{
		auto before_best_bid = get_best_price(_bid_book);
		auto before_best_ask = get_best_price(_ask_book);
		if (handle_cancel(o))
		{
			auto after_best_bid = get_best_price(_bid_book);
			auto after_best_ask = get_best_price(_ask_book);
			handle_stop(before_best_bid,
					before_best_ask,
					after_best_bid,
					after_best_ask);
		}
	}
	else
	{
		auto before_best_bid = get_best_price(_bid_book);
		auto before_best_ask = get_best_price(_ask_book);
		if (handle_amend(o))
		{
			auto after_best_bid = get_best_price(_bid_book);
			auto after_best_ask = get_best_price(_ask_book);
			handle_stop(before_best_bid,
					before_best_ask,
					after_best_bid,
					after_best_ask);
		}
	}
}




