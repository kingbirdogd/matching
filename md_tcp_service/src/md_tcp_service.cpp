#include <md_tcp_service.hpp>
#include <iostream>



md_tcp_service::md_tcp_service
(
	const std::string outright_ip,
	unsigned short int outright_port,
	unsigned short int bind_port,
	const std::string bind_ip,
	unsigned long long mtick,
	unsigned short int xsub_port
):
_services(),
_outright(outright_ip, outright_port),
_book([&](const md::book_item& item){_handle_item(item);},mtick),
_s(bind_port, bind_ip),
_outright_port(outright_port)
{
	_outright.set_on_order([&](const matching::order& o)
	{
		_book.handle_outright(o);
	});
	_s.set_on_connect([&](net::tcp_client* cli)
	{
		_book.recovery([&, cli](const md::book_item& item)
		{
			cli->send(item);
		});
	});
	std::stringstream url;
	url << "tcp://localhost:" << xsub_port;
}

void md_tcp_service::_handle_item(const md::book_item& item)
{
  std::cout << "side=" << item.side << ",px=" << item.price << ",qty=" << item.quantity << std::endl;
	_s.send(item);
}

void md_tcp_service::add_implied_service
(
	const std::string a_ip,
	unsigned short int a_port,
	const std::string b_ip,
	unsigned short int b_port,
	md::md_implied_book::implier_type bid_implie_type,
	md::md_implied_book::implier_type ask_implie_type,
	implier* bid_implier,
	implier* ask_implier
)
{
	_services.push_back(implied_md_tcp_service(_book,
			a_ip, a_port,
			b_ip, b_port,
			bid_implie_type, ask_implie_type,
			bid_implier, ask_implier));
}

void md_tcp_service::run()
{
	_outright.run();
	for (auto& isrv : _services)
		isrv.run();
	_s.run();
}





