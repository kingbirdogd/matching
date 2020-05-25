#ifndef NET_ZMQ_SERVER_INC_ZMQ_SERVICE_HPP_
#define NET_ZMQ_SERVER_INC_ZMQ_SERVICE_HPP_

#include <unordered_set>
#include <functional>
#include <net/zmq_client.hpp>

namespace net
{
	class zmq_service
	{
	private:
		enum class status : unsigned int
		{
			UNBIND = 0x00,
			BINDED = 0x01
		};
		using client_set = std::unordered_set<zmq_client*>;
		using msg_cb = std::function<void(zmq_client*, const char*, std::size_t)>;
		using service_event_cb = std::function<void()>;
		using client_event_cb = std::function<void(zmq_client*)>;
	private:
		service_event_cb _on_bind;
		service_event_cb _on_unbind;
		client_event_cb _on_connected;
		client_event_cb _on_disconnected;
		msg_cb _on_msg;
		client_set _clients;
		int _sock;
		std::string _bind_addr;
		unsigned short int _bind_port;
		status _sta;
    zmq::context_t &_ctx;
    zmq::socket_t _pub_sock;
		zmq::socket_t _rep_sock;
	public:
		zmq_service(zmq::context_t &ctx, unsigned short int bind_port, const std::string& bind_addr = "");
    //zmq_service(zmq::context_t &ctx);
		zmq_service(zmq_service&&);
		//zmq_service& operator=(zmq_service&&);
		~zmq_service();
		bool send(zmq_client* cli, const void* ptr, std::size_t size);
		void send(const void* ptr, std::size_t size);
		template <typename T>
		void send(zmq_client* cli, const T& obj)
		{
			send(cli, &obj, sizeof(T));
		}
		template <typename T>
		void send(const T& obj)
		{
			send(&obj, sizeof(T));
		}
		bool close(zmq_client* cli);
		void close();
		void set_on_msg(msg_cb&& _on_msg);
		void set_service_bind(service_event_cb&& on_bind);
		void set_service_unbind(service_event_cb&& on_unbind);
		void set_on_connect(client_event_cb&& on_connected);
		void set_on_disconnect(client_event_cb&& on_disconnected);
		void run();
		zmq_service() = delete;
		zmq_service(const zmq_client&) = delete;
		zmq_service& operator=(const zmq_client&) = delete;
	private:
		void _bind();
		void _accept();
		void _run_clients();
	};
}



#endif /* NET_ZMQ_SERVER_INC_ZMQ_SERVICE_HPP_ */
