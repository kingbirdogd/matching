#include <matching/engine.hpp>
#include <ctime>

using namespace matching;

std::atomic<unsigned long long> engine::_id(static_cast<unsigned long long>(time(nullptr)) * 1000000000);

engine::engine(callback_type&& callback):
		_odr_map(),
		_bid_book(),
		_ask_book(),
		_stop_book(),
		_bid_stop_book(),
		_ask_stop_book(),
		_callback(std::move(callback))
{
}

engine::~engine()
{
}

void engine::handle(order& o)
{
	o.order_id = get_id();
	if (order::order_action_type::NEW == o.order_action)
	{
		handle_new(o);
	}
	else if (order::order_action_type::CANCEL == o.order_action)
	{
		handle_cancel(o);
	}
	else
	{
		handle_amend(o);
	}
}




