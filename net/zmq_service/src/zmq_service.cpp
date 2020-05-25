#include "../../zmq_service/inc/net/zmq_service.hpp"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>

using namespace net;


//zmq_service::zmq_service(unsigned short int bind_port, const std::string& bind_addr):
zmq_service::zmq_service(zmq::context_t &ctx, unsigned short int bind_port, const std::string& bind_addr):
		_on_bind(),
		_on_unbind(),
		_on_connected(),
		_on_disconnected(),
		_on_msg(),
		_clients(),
		_sock(0),
		_bind_addr(bind_addr),
		_bind_port(bind_port),
		_sta(status::UNBIND),
		_ctx(ctx),
    _pub_sock(_ctx, ZMQ_PUB)
{
  //_pub_sock.connect("tcp://localhost:14002"); // hard code for testing
  //std::cout << "Connected to xsub" << std::endl;
}

zmq_service::zmq_service(zmq_service&& s):
		_on_bind(std::move(s._on_bind)),
		_on_unbind(std::move(s._on_unbind)),
		_on_connected(std::move(s._on_connected)),
		_on_disconnected(std::move(s._on_disconnected)),
		_on_msg(std::move(s._on_msg)),
		_clients(std::move(s._clients)),
		_sock(s._sock),
		_bind_addr(std::move(s._bind_addr)),
		_bind_port(s._bind_port),
		_sta(s._sta),
		_ctx(s._ctx),
		_pub_sock(_ctx, ZMQ_PUB)
{
}
/*
zmq_service& zmq_service::operator=(zmq_service&& s)
{
	_on_bind = std::move(s._on_bind);
	_on_unbind = std::move(s._on_unbind);
	_on_connected = std::move(s._on_connected);
	_on_disconnected = std::move(s._on_disconnected);
	_on_msg = std::move(s._on_msg);
	_sock = s._sock;
	_clients = std::move(s._clients);
	//_bind_addr = std::move(s._bind_addr);
	//_bind_port = s._bind_port;
	_sta = s._sta;
  _ctx = s._ctx;
	return *this;
}
*/
zmq_service::~zmq_service()
{
	close();
}

bool zmq_service::send(zmq_client* cli, const void* ptr, std::size_t size)
{
	if (_clients.end() == _clients.find(cli))
	{
		return false;
	}
	cli->send(ptr, size);
	return true;
}


void zmq_service::send(const void* ptr, std::size_t size)
{
	for (auto it = _clients.begin(); it !=  _clients.end(); ++it)
	{
		auto cli = (*it);
		cli->send(ptr, size);
	}
  //_pub_sock.send(zmq::const_buffer(ptr,  size), zmq::send_flags::none);
}

bool zmq_service::close(zmq_client* cli)
{
	auto it = _clients.find(cli);
	if (_clients.end() == it)
		return false;
	_clients.erase(it);
	if (_on_disconnected)
	{
		_on_disconnected(cli);
	}
	delete cli;
	return true;
}

void zmq_service::close()
{
	if (status::BINDED == _sta)
	{
		auto it = _clients.begin();
		while (it != _clients.end())
		{
			auto ptr = (*it);
			it = _clients.erase(it);
			if (_on_disconnected)
			{
				_on_disconnected(ptr);
			}
			delete ptr;
		}
		_sta = status::UNBIND;
		if (_on_unbind)
		{
			_on_unbind();
		}
	}
}

void zmq_service::set_on_msg(msg_cb&& on_msg)
{
	_on_msg = std::move(on_msg);
}

void zmq_service::set_service_bind(service_event_cb&& on_bind)
{
	_on_bind = std::move(on_bind);
}

void zmq_service::set_service_unbind(service_event_cb&& on_unbind)
{
	_on_unbind = std::move(on_unbind);
}

void zmq_service::set_on_connect(client_event_cb&& on_connected)
{
	_on_connected = std::move(on_connected);
}

void zmq_service::set_on_disconnect(client_event_cb&& on_disconnected)
{
	_on_disconnected = std::move(on_disconnected);
}

void zmq_service::run()
{
	if (status::UNBIND == _sta)
		_bind();
	else
	{
		_accept();
		_run_clients();
	}
}

void zmq_service::_bind()
{
  std::stringstream bind_url;
  bind_url << "tcp://*:" << _bind_port;
  _rep_sock.bind(bind_url.str().c_str());
/*
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_bind_port);
	if (_bind_addr == "")
	{
		addr.sin_addr.s_addr = INADDR_ANY;
	}
	else if (::inet_pton(AF_INET, _bind_addr.c_str(), &addr.sin_addr) <= 0)
	{
		return;
	}
	bzero(&(addr.sin_zero), 8);
	auto fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == fd)
		return;
	if (-1 == ::bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)))
	{
		::close(fd);
		return;
	}
	auto flags = ::fcntl(fd, F_GETFL, 0);
	if (flags < 2)
	{
		::close(fd);
		return;
	}
	if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		::close(fd);
		return;
	}
	if (::listen(fd, SOMAXCONN) < 0)
	{
		::close(fd);
		return;
	}
*/
	_sta = status::BINDED;
//_sock = fd;
	if (_on_bind)
	{
		_on_bind();
	}
}

void zmq_service::_accept()
{
  /*
	struct sockaddr addr;
	socklen_t len = sizeof(addr);
	int fd = ::accept(_sock, &addr, &len);
	if (fd < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			close();
		}
		return;
	}
	auto flags = ::fcntl(fd, F_GETFL, 0);
	if (flags < 2)
	{
		::close(fd);
		return;
	}
	if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		::close(fd);
		return;
	}
	struct sockaddr_in* v4addr = (struct sockaddr_in*)&addr;
	struct in_addr ipaddr = v4addr->sin_addr;
	char remote_addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ipaddr, remote_addr, INET_ADDRSTRLEN);
	unsigned short int remote_port = v4addr->sin_port;
	remote_port = ntohs(remote_port);
	auto cli = new zmq_client(fd, remote_addr, remote_port);
	if (_on_connected)
	{
		_on_connected(cli);
	}
	_clients.insert(cli);
	cli->set_disconnected([&, this, cli]()
	{
		auto it = _clients.find(cli);
		if (_clients.end() != it)
		{
			if (_on_disconnected)
			{
				_on_disconnected(cli);
			}
		}
	});
 */
//	cli->set_on_msg([&, cli](const char* ptr, std::size_t size)
//	{
//		if (_on_msg)
//		{
//			_on_msg(cli, ptr, size);
//		}
//	});
}

void zmq_service::_run_clients()
{
	for (auto it = _clients.begin(); it != _clients.end();)
	{
		auto ptr = *it;
		if (ptr->run())
			++it;
		else
			it = _clients.erase(it);
	}
}



