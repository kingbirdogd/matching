#ifndef MATCHING_TCP_CLIENT_INC_MATCHING_TCP_CLIENT_HPP_
#define MATCHING_TCP_CLIENT_INC_MATCHING_TCP_CLIENT_HPP_

#include <matching/order.hpp>
#include <matching/order_handler.hpp>
#include <net/tcp_client.hpp>
#include <functional>
#include <queue>
#include <mutex>

class matching_tcp_client
{
private:
	using snd_queue = std::queue<matching::order>;
	using control_event = std::function<void()>;
	using order_event = std::function<void(const matching::order&)>;
private:
	snd_queue q_;
	net::tcp_client c_;
	matching::order_handler h_;
	control_event _on_connected;
	control_event _on_disconnected;
	order_event _on_order;
	std::mutex m_;
	bool valid_;
public:
	matching_tcp_client(const std::string& host, unsigned short int port);
	matching_tcp_client(matching_tcp_client&& c);
	matching_tcp_client& operator= (matching_tcp_client&& c);
	~matching_tcp_client() = default;
	void send(const matching::order& o);
	void run();
	void set_connected(control_event&& on_connected);
	void set_disconnected(control_event&& on_disconnected);
	void set_on_order(order_event&& on_order);
	matching_tcp_client() = delete;
	matching_tcp_client(const matching_tcp_client&) = delete;
	matching_tcp_client& operator= (const matching_tcp_client&) = delete;
	operator bool() const;
private:
	void _handle_rcv_odr(const matching::order& o);
	void _send_odrs();
};



#endif /* MATCHING_TCP_CLIENT_INC_MATCHING_TCP_CLIENT_HPP_ */
