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

md_tcp_service::~md_tcp_service()
{
	for (auto& isrv : _services)
		delete isrv;
}

std::string time_in_HH_MM_SS_MMM()
{
  using namespace std::chrono;

  // get current time
  auto now = system_clock::now();

  // get number of milliseconds for the current second
  // (remainder after division into seconds)
  auto us = duration_cast<microseconds>(now.time_since_epoch()) % 1000000;

  // convert to std::time_t in order to convert to std::tm (broken time)
  auto timer = system_clock::to_time_t(now);

  // convert to broken time
  std::tm bt = *std::localtime(&timer);

  std::ostringstream oss;

  oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S"); // HH:MM:SS
  oss << '.' << std::setfill('0') << std::setw(6) << us.count();

  return oss.str();
}

void md_tcp_service::_handle_item(const md::book_item& item)
{
  std::clog << time_in_HH_MM_SS_MMM() << " side=" << item.side << ",px=" << item.price << ",qty=" << item.quantity << std::endl;
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
	_services.push_back(new implied_md_tcp_service(_book,
			a_ip, a_port,
			b_ip, b_port,
			bid_implie_type, ask_implie_type,
			bid_implier, ask_implier));
}

void md_tcp_service::run()
{
	_outright.run();
	for (auto& isrv : _services)
		isrv->run();
	_s.run();
}





