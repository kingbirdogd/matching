#include <engine.hpp>
#include <ctime>

using namespace matching;

std::atomic<unsigned long long> engine::_id(static_cast<unsigned long long>(time(nullptr)) * 1000000000);

engine::engine(callback_type&& callback):
		_bid_book(),
		_ask_book(),
		_callback(std::move(callback))
{
}


engine::~engine()
{
}




