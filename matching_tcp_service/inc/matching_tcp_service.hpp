#ifndef MATCHING_TCP_SERVICE_INC_MATCHING_TCP_SERVICE_HPP_
#define MATCHING_TCP_SERVICE_INC_MATCHING_TCP_SERVICE_HPP_

#include <matching/engine.hpp>
#include <matching/order_handler.hpp>
#include "../../net/tcp_service/inc/net/tcp_service.hpp"


class matching_tcp_service
{
private:
	matching::engine e_;
	matching::order_handler h_;
	net::tcp_service s_;
public:
	matching_tcp_service(unsigned short int bind_port, const std::string& bind_addr = "");
	matching_tcp_service(matching_tcp_service&& s);
	matching_tcp_service& operator=(matching_tcp_service&& s);
	~matching_tcp_service() = default;
	matching_tcp_service() = delete;
	matching_tcp_service(const matching_tcp_service&) = delete;
	matching_tcp_service& operator=(const matching_tcp_service&) = delete;
	void run();
	matching::engine& get_engine();
private:
	void _handle_rcv_odr(const matching::order& o);
	void _handle_snd_odr(const matching::order& o);
};



#endif /* MATCHING_TCP_SERVICE_INC_MATCHING_TCP_SERVICE_HPP_ */
