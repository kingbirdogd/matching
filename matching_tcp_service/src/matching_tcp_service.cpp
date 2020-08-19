#include <matching_tcp_service.hpp>

matching_tcp_service::matching_tcp_service(
    unsigned long long tick_sz,
    unsigned short int bind_port,
		const std::string& bind_addr):
		e_([&](const matching::order& o){_handle_snd_odr(o);}, tick_sz),
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
  fprintf(stderr, "%lu Received ORDER. timestamp:%llu market_id:%llu request_id:%llu client_order_id:%llu action:%d order_id:%llu\n",
          current(), o.timestamp_epoch_ms, o.market_id, o.request_id, o.client_order_id, (int)o.order_action, o.order_id);
	e_.handle(const_cast<matching::order&>(o));
  fprintf(stderr, "%lu Handled  ORDER\n", current());
}

void matching_tcp_service::_handle_snd_odr(const matching::order& o)
{
  fprintf(stderr, "%lu Sending  REPLY. timestamp:%llu market_id:%llu request_id:%llu client_order_id:%llu action:%d order_id:%llu\n",
          current(), o.timestamp_epoch_ms, o.market_id, o.request_id, o.client_order_id, (int)o.order_action, o.order_id);
  s_.send(o);
  fprintf(stderr, "%lu Sent     REPLY. timestamp:%llu market_id:%llu request_id:%llu client_order_id:%llu action:%d order_id:%llu\n",
          current(), o.timestamp_epoch_ms, o.market_id, o.request_id, o.client_order_id, (int)o.order_action, o.order_id);
}

matching::engine& matching_tcp_service::get_engine()
{
	return e_;
}


