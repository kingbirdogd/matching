#include <matching_zmq_service.hpp>

matching_zmq_service::matching_zmq_service(
    zmq::context_t &ctx,
    unsigned short int bind_port,
		const std::string& bind_addr
		):
		e_([&](const matching::order& o){_handle_snd_odr(o);}),
		h_([&](const matching::order& o){_handle_rcv_odr(o);}),
		s_(ctx, bind_port, bind_addr)
		//s_(bind_port, bind_addr)
{
	s_.set_on_connect([&](net::zmq_client* cli)
	{
		e_.recovery([&, cli](const matching::order& o)
		{
			cli->send(o);
		});
	});
	s_.set_on_msg([&](net::zmq_client*, const char* ptr, std::size_t size)
	{
		h_.handle(ptr, size);
	});
}

matching_zmq_service::matching_zmq_service(matching_zmq_service&& s):
		e_(std::move(s.e_)),
		h_(std::move(s.h_)),
		s_(std::move(s.s_))
{
}
/*
matching_zmq_service& matching_zmq_service::operator=(matching_zmq_service&& s)
{
	e_ = std::move(s.e_);
	h_ =std::move(s.h_);
	s_ = std::move(s.s_);
	return *this;
}
*/
void matching_zmq_service::run()
{
	s_.run();
}

void matching_zmq_service::_handle_rcv_odr(const matching::order& o)
{
	e_.handle(const_cast<matching::order&>(o));
}

void matching_zmq_service::_handle_snd_odr(const matching::order& o)
{
	s_.send(o);
}

matching::engine& matching_zmq_service::get_engine()
{
	return e_;
}


