#include <matching_tcp_client.hpp>

matching_tcp_client::matching_tcp_client(const std::string& host, unsigned short int port):
	q_(),
	c_(host, port),
	h_([&](const matching::order& o){_handle_rcv_odr(o);}),
	_on_connected(),
	_on_disconnected(),
	_on_order(),
	m_()
{
	c_.set_connected([&]()
	{
		if (_on_connected)
			_on_connected();
	});
	c_.set_disconnected([&]()
	{
		if (_on_disconnected)
			_on_disconnected();
	});
	c_.set_on_msg([&](const char* ptr, std::size_t size)
	{
		h_.handle(ptr, size);
	});
}

matching_tcp_client::matching_tcp_client(matching_tcp_client&& c):
	q_(std::move(c.q_)),
	c_(std::move(c.c_)),
	h_(std::move(c.h_)),
	_on_connected(std::move(c._on_connected)),
	_on_disconnected(std::move(c._on_disconnected)),
	_on_order(std::move(c._on_order)),
	m_()
{
}

matching_tcp_client& matching_tcp_client::operator= (matching_tcp_client&& c)
{
	q_ = std::move(c.q_);
	c_ = std::move(c.c_);
	h_ = std::move(c.h_);
	_on_connected = std::move(c._on_connected);
	_on_disconnected = std::move(c._on_disconnected);
	_on_order = std::move(c._on_order);
	return *this;
}

void matching_tcp_client::send(const matching::order& o)
{
	std::lock_guard<std::mutex> l(m_);
	q_.push(o);
}

void matching_tcp_client::run()
{
	_send_odrs();
	c_.run();
}

void matching_tcp_client::set_connected(control_event&& on_connected)
{
	_on_connected = std::move(on_connected);
}

void matching_tcp_client::set_disconnected(control_event&& on_disconnected)
{
	_on_disconnected = std::move(on_disconnected);
}

void matching_tcp_client::set_on_order(order_event&& on_order)
{
	_on_order = std::move(on_order);
}

void matching_tcp_client::_handle_rcv_odr(const matching::order& o)
{
	if (_on_order)
		_on_order(o);
}

void matching_tcp_client::_send_odrs()
{
	std::lock_guard<std::mutex> l(m_);
	while (!q_.empty())
	{
		auto& o = q_.front();
		c_.send(o);
		q_.pop();
	}
}




