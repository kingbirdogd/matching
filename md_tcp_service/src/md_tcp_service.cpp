#include <md_tcp_service.hpp>
#include <iostream>



md_tcp_service::md_tcp_service
(
	const std::string outright_ip,
	unsigned short int outright_port,
	const std::string a_ip,
	unsigned short int a_port,
	const std::string b_ip,
	unsigned short int b_port,
	md::md_book::implier_type bid_implie_type,
	md::md_book::implier_type ask_implie_type,
	implier* bid_implier,
	implier* ask_implier,
	unsigned short int bind_port,
	const std::string bind_ip,
	unsigned long long mtick,
  unsigned short int xsub_port
):
	_outright(outright_ip, outright_port),
	_a(a_ip, a_port),
	_b(b_ip, b_port),
	_book([&](const md::book_item& item){_handle_item(item);},
			mtick,
			bid_implie_type,
			ask_implie_type,
			bid_implier,
			ask_implier),
	_s(bind_port, bind_ip),
  _outright_port(outright_port)
{
	_outright.set_on_order([&](const matching::order& o)
	{
		_book.handle_outright(o);
	});
	_a.set_on_order([&](const matching::order& o)
	{
		_book.handle_a(o);
	});
	_b.set_on_order([&](const matching::order& o)
	{
		_book.handle_b(o);
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

  std::cout << time_in_HH_MM_SS_MMM() << " side=" << item.side << ",px=" << item.price << ",qty=" << item.quantity << '\n';
	_s.send(item);
}

void md_tcp_service::run()
{
	_outright.run();
	_a.run();
	_b.run();
	_s.run();
}





