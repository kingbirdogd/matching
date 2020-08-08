#include <net/tcp_service.hpp>
#include <net/tcp_client.hpp>
#include <rapid_ring/ring_buffer_queue.hpp>
#include <matching_tcp_client.hpp>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <thread>
#include <iostream>

using out_buffer = std::vector<char>;

struct odr_query_request
{
	unsigned long long account_id;
	unsigned long long market_id;
	unsigned long long request_id;
};

struct odr_query_response_header
{
	unsigned long long account_id;
	unsigned long long market_id;
	unsigned long long request_id;
	unsigned long long odr_cnt;
};

struct odr_query_tcp_request
{
	odr_query_request request;
	net::tcp_client* client;
};

struct order_query_tcp_response
{
	net::tcp_client* client;
	out_buffer* buff_ptr;
};

using request_queue = rapid_ring::spsc_ring_buffer_queue<odr_query_tcp_request, 8192>;
using response_queue = rapid_ring::mpsc_ring_buffer_queue<order_query_tcp_response, 8192>;

class odr_query_client
{
private:
	using account_order_map = std::unordered_map<unsigned long long, matching::order>;
private:
	response_queue& _response_queue;
	matching_tcp_client _cli;
	account_order_map _map;
	request_queue _request_queue;
	std::thread _thread;
	bool _is_start;
private:
	void handle_odr(const matching::order& odr)
	{
		if (matching::order::order_status_type::OPEN != odr.order_state
				&& matching::order::order_status_type::PARTIAL_FILL)
		{
			_map[odr.account_id] = odr;
		}
		else
		{
			_map.erase(odr.account_id);
		}
	}
	void handle_request(const odr_query_tcp_request& request)
	{
		order_query_tcp_response rsp;
		rsp.client = request.client;
		out_buffer* buff = new out_buffer();
		rsp.buff_ptr = buff;
		std::size_t buff_size = sizeof(odr_query_response_header) + sizeof(matching::order) * _map.size();
		buff->resize(buff_size);
		auto ptr = &(buff->at(0));
		std::memcpy(ptr, &(request.request), sizeof(request.request));
		auto rsp_ptr = static_cast<odr_query_response_header*>(static_cast<void*>(ptr));
		rsp_ptr->odr_cnt = _map.size();
		ptr += sizeof(odr_query_response_header);
		for (const auto& item : _map)
		{
			std::memcpy(ptr, &(item.second), sizeof(matching::order));
		}
		_response_queue.enqueue(rsp);
	}
	void run()
	{
		while (_is_start)
		{
			_cli.run();
			odr_query_tcp_request request;
			if (_request_queue.try_dequeue(request))
			{
				handle_request(request);
			}
		}
	}
public:
	request_queue& get_request_queue()
	{
		return _request_queue;
	}
	odr_query_client() = delete;
	odr_query_client(const odr_query_client& cli) = delete;
	odr_query_client& operator= (const odr_query_client& cli) = delete;
	odr_query_client(odr_query_client&& cli):
		_response_queue(cli._response_queue),
		_cli(std::move(cli._cli)),
		_map(std::move(cli._map)),
		_request_queue(),
		_thread(std::move(cli._thread)),
		_is_start(cli._is_start)
	{
	}
	odr_query_client& operator= (odr_query_client&& cli)
	{
		_cli = std::move(cli._cli);
		_map = std::move(cli._map);
		_thread = std::move(cli._thread);
		_is_start = cli._is_start;
		return *this;
	}
	~odr_query_client()
	{
		if (_is_start)
		{
			_is_start = false;
			_thread.join();
		}
	}
	odr_query_client(response_queue& rsp, std::string ip, unsigned short int port):
		_response_queue(rsp),
		_cli(ip, port),
		_map(),
		_request_queue(),
		_thread(),
		_is_start(false)
	{
		_cli.set_on_order([&](const matching::order& odr)
		{
			handle_odr(odr);
		});
	}
	void start()
	{
		_is_start = true;
		_thread = std::thread([&]()
		{
			run();
		});
	}
	void stop()
	{
		if (_is_start)
		{
			_is_start = false;
			_thread.join();
		}
	}
};

class odr_query_server
{
private:
	struct request_buffer
	{
		char buff[sizeof(odr_query_request)];
		std::size_t rest;
		request_buffer():
			buff{},
			rest(0)
		{
		}
		~request_buffer() = default;
		request_buffer(const request_buffer& req):
			buff{},
			rest(req.rest)
		{
			std::memcpy(buff, req.buff, sizeof(buff));
		}
		request_buffer(request_buffer&& req):
			buff{},
			rest(req.rest)
		{
			std::memcpy(buff, req.buff, sizeof(buff));
		}
		request_buffer& operator= (const request_buffer& req)
		{
			std::memcpy(buff, req.buff, sizeof(buff));
			return *this;
		}
		request_buffer& operator= (request_buffer&& req)
		{
			std::memcpy(buff, req.buff, sizeof(buff));
			return *this;
		}
	};
private:
	using client_buffer_map = std::unordered_map<net::tcp_client*, request_buffer>;
	using market_map = std::unordered_map<unsigned long long, odr_query_client>;
private:
	client_buffer_map _cli_map;
	market_map _mkt_map;
	net::tcp_service _srv;
	response_queue _response_queue;
public:
	odr_query_server() = delete;
	~odr_query_server() = default;
	odr_query_server(const odr_query_server&) = delete;
	odr_query_server& operator=(const odr_query_server&) = delete;
	odr_query_server(odr_query_server&& srv):
		_cli_map(std::move(srv._cli_map)),
		_mkt_map(std::move(srv._mkt_map)),
		_srv(std::move(srv._srv)),
		_response_queue()

	{
	}
	odr_query_server& operator=(odr_query_server&& srv)
	{
		_cli_map = std::move(srv._cli_map);
		_mkt_map = std::move(srv._mkt_map);
		_srv = std::move(srv._srv);
		return *this;
	}
	odr_query_server(unsigned short int port, const std::string ip = ""):
		_cli_map(),
		_mkt_map(),
		_srv(port, ip),
		_response_queue()
	{
		_srv.set_on_disconnect([&](net::tcp_client* cli)
		{
			_cli_map.erase(cli);
		});
		_srv.set_on_msg([&](net::tcp_client* cli, const char* ptr, std::size_t size)
		{
			auto& item = _cli_map[cli];
			if (0 != item.rest)
			{
				auto need = sizeof(odr_query_request) - item.rest;
				if (size < need)
				{
					std::memcpy(&item.buff[item.rest], ptr, size);
					item.rest += size;
					return;
				}
				else
				{
					std::memcpy(&item.buff[item.rest], ptr, need);
					item.rest = 0;
					size -= need;
					ptr += need;
					odr_query_tcp_request req;
					std::memcpy(&req.request, item.buff, sizeof(req.request));
					req.client = cli;
					auto it = _mkt_map.find(req.request.market_id);
					if (_mkt_map.end() == it)
					{
						odr_query_response_header error_rsp;
						error_rsp.account_id = req.request.account_id;
						error_rsp.market_id = req.request.market_id;
						error_rsp.request_id = req.request.request_id;
						error_rsp.odr_cnt = 0;
						_srv.send(req.client, error_rsp);
					}
					else
					{
						auto& request_queue = it->second.get_request_queue();
						request_queue.enqueue(req);
					}
				}
			}
			auto msg_cnt = size / sizeof(odr_query_request);
			for (std::size_t i = 0; i < msg_cnt; ++i)
			{
				odr_query_tcp_request req;
				std::memcpy(&req.request, item.buff, sizeof(req.request));
				req.client = cli;
				auto it = _mkt_map.find(req.request.market_id);
				if (_mkt_map.end() == it)
				{
					odr_query_response_header error_rsp;
					error_rsp.account_id = req.request.account_id;
					error_rsp.market_id = req.request.market_id;
					error_rsp.request_id = req.request.request_id;
					error_rsp.odr_cnt = 0;
					_srv.send(cli, error_rsp);
				}
				else
				{
					auto& request_queue = it->second.get_request_queue();
					request_queue.enqueue(req);
				}
				ptr += sizeof(odr_query_request);
				size -= sizeof(odr_query_request);
			}
			if (size > 0)
			{
				std::memcpy(item.buff, ptr, size);
				item.rest = size;
			}
		});
	}
	bool add_mkt(unsigned long long mkt_id, const std::string&ip, unsigned short int port)
	{
		if(_mkt_map.end() != _mkt_map.find(mkt_id))
		{
			return false;
		}
		auto& cli = _mkt_map.emplace(mkt_id, odr_query_client(_response_queue, ip, port)).first->second;
		cli.start();
		return true;
	}

	bool remove_mkt(unsigned long long mkt_id)
	{
		if(_mkt_map.end() == _mkt_map.find(mkt_id))
		{
			return false;
		}
		_mkt_map.erase(mkt_id);
		return true;
	}
	void run()
	{
		_srv.run();
		order_query_tcp_response rsp;
		if (_response_queue.try_dequeue(rsp))
		{
			auto& buff = *rsp.buff_ptr;
			_srv.send(rsp.client, &buff[0], buff.size());
			delete rsp.buff_ptr;
		}
	}
};

int main(int iArgc, char** pszArgv)
{
	auto arg_cnt = iArgc  - 1;
	auto mode = arg_cnt % 3;
	auto mkt_cnt = arg_cnt / 3;
	if ((1 != mode && 2 != mode) || 0 == mkt_cnt)
	{
		std::cerr << "usage odr_query_service "
						"<service_port>"
						"[service_ip] "
						"<market_id> "
						"<market_ip> "
						"<market_port> "
						"<market_id> "
						"<market_ip> "
						"<market_port> "
						"..."
						<< std::endl;
				return -1;
	}
	auto service_port = static_cast<unsigned short int>(std::stoi(pszArgv[1]));
	std::string service_ip = "";
	std::size_t idx = 2;
	if (2 == mode)
	{
		service_ip = pszArgv[2];
		idx = 3;
	}
	odr_query_server srv(service_port, service_ip);
	for (int i = 0; i < mkt_cnt; ++i)
	{
		unsigned long long mkt_id = std::stoull(pszArgv[idx]);
		std::string mkt_ip = pszArgv[idx + 1];
		auto mkt_port = static_cast<unsigned short int>(std::stoi(pszArgv[idx + 2]));
		srv.add_mkt(mkt_id, mkt_ip, mkt_port);
		idx += 3;
	}
	while(true)
		srv.run();
	return 0;
}




