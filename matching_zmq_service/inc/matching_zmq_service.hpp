#ifndef MATCHING_ZMQ_SERVICE_INC_MATCHING_ZMQ_SERVICE_HPP_
#define MATCHING_ZMQ_SERVICE_INC_MATCHING_ZMQ_SERVICE_HPP_

#include <matching/engine.hpp>
#include <matching/order_handler.hpp>
#include "../../net/zmq_service/inc/net/zmq_service.hpp"
#include <zmq.hpp>
//#include "../../net/tcp_service/inc/net/tcp_service.hpp"


class matching_zmq_service
{
private:
	matching::engine e_;
	matching::order_handler h_;
	net::zmq_service s_;
public:
	//matching_zmq_service(unsigned short int bind_port, const std::string& bind_addr = "");
  matching_zmq_service(zmq::context_t &ctx, unsigned short int bind_port, const std::string& bind_addr = "");
	matching_zmq_service(matching_zmq_service&& s);
	//matching_zmq_service& operator=(matching_zmq_service&& s);
	~matching_zmq_service() = default;
	matching_zmq_service() = delete;
	matching_zmq_service(const matching_zmq_service&) = delete;
	matching_zmq_service& operator=(const matching_zmq_service&) = delete;
	void run();
	matching::engine& get_engine();
private:
	void _handle_rcv_odr(const matching::order& o);
	void _handle_snd_odr(const matching::order& o);
};



#endif /* MATCHING_ZMQ_SERVICE_INC_MATCHING_ZMQ_SERVICE_HPP_ */
