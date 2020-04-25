#include <matching_tcp_service.hpp>

matching_tcp_service::matching_tcp_service(unsigned short int bind_port,
		const std::string& bind_addr):
		e_([&](const matching::order& o){_handle_snd_odr(o);}),
		h_([&](const matching::order& o){_handle_rcv_odr(o);}),
		s_(bind_port, bind_addr)
{
	s_.set_on_connect([&](net::tcp_client* cli)
	{
		e_.recovery([&, cli](const matching::order& o)
		{
			cli->send(o);
		});
	});
	s_.set_on_msg([&](net::tcp_client*, const char* ptr, std::size_t size)
	{
		h_.handle(ptr, size);
	});
}

matching_tcp_service::matching_tcp_service(matching_tcp_service&& s):
		e_(std::move(s.e_)),
		h_(std::move(s.h_)),
		s_(std::move(s.s_))
{
}

matching_tcp_service& matching_tcp_service::operator=(matching_tcp_service&& s)
{
	e_ = std::move(s.e_);
	h_ =std::move(s.h_);
	s_ = std::move(s.s_);
	return *this;
}

void matching_tcp_service::run()
{
	s_.run();
}

void matching_tcp_service::_handle_rcv_odr(const matching::order& o)
{
	e_.handle(const_cast<matching::order&>(o));
}

void matching_tcp_service::_handle_snd_odr(const matching::order& o)
{
	s_.send(o);
}


