#include <net/tcp_service.hpp>

using namespace net;


tcp_service::tcp_service(unsigned short int bind_port, const std::string& bind_addr):
		_on_bind(),
		_on_unbind(),
		_on_connected(),
		_on_disconnected(),
		_on_msg(),
		_clients(),
		_sock(0),
		_bind_addr(bind_addr),
		_bind_port(bind_port),
		_sta(status::UNBIND)
{
}

tcp_service::tcp_service(tcp_service&& s):
		_on_bind(std::move(s._on_bind)),
		_on_unbind(std::move(s._on_unbind)),
		_on_connected(std::move(s._on_connected)),
		_on_disconnected(std::move(s._on_disconnected)),
		_on_msg(std::move(s._on_msg)),
		_clients(std::move(s._clients)),
		_sock(s._sock),
		_bind_addr(std::move(s._bind_addr)),
		_bind_port(s._bind_port),
		_sta(s._sta)
{
}

tcp_service& tcp_service::operator=(tcp_service&& s)
{
	_on_bind = std::move(s._on_bind);
	_on_unbind = std::move(s._on_unbind);
	_on_connected = std::move(s._on_connected);
	_on_disconnected = std::move(s._on_disconnected);
	_on_msg = std::move(s._on_msg);
	_sock = s._sock;
	_clients = std::move(s._clients);
	_bind_addr = std::move(s._bind_addr);
	_bind_port = s._bind_port;
	_sta = s._sta;
	return *this;
}

tcp_service::~tcp_service()
{
	close();
}

bool tcp_service::send(tcp_client* cli, const char* ptr, std::size_t size)
{
	if (_clients.end() == _clients.find(cli))
	{
		return false;
	}
	cli->send(ptr, size);
	return true;
}


void tcp_service::send(const char* ptr, std::size_t size)
{
	for (auto it = _clients.begin(); it !=  _clients.end(); ++it)
	{
		auto cli = (*it);
		cli->send(ptr, size);
	}
}

bool tcp_service::close(tcp_client* cli)
{
	auto it = _clients.find(cli);
	if (_clients.end() == it)
		return false;
	_clients.erase(it);
	delete cli;
	return true;
}

void tcp_service::close()
{
	if (status::BINDED == _sta)
	{
		auto it = _clients.begin();
		while (it != _clients.end())
		{
			auto ptr = (*it);
			it = _clients.erase(it);
			delete ptr;
		}
		_sta = status::UNBIND;
	}
}

void tcp_service::set_on_msg(msg_cb&& on_msg)
{
	_on_msg = std::move(on_msg);
}

void tcp_service::set_service_bind(service_event_cb&& on_bind)
{
	_on_bind = std::move(on_bind);
}

void tcp_service::set_service_unbind(service_event_cb&& on_unbind)
{
	_on_unbind = std::move(on_unbind);
}

void tcp_service::set_on_connect(client_event_cb&& on_connected)
{
	_on_connected = std::move(on_connected);
}

void tcp_service::set_on_disconnect(client_event_cb&& on_disconnected)
{
	_on_disconnected = std::move(on_disconnected);
}

void tcp_service::run()
{
	if (status::UNBIND == _sta)
		bind();
	else
		accept();
}

void tcp_service::bind()
{
}

void tcp_service::accept()
{
}



