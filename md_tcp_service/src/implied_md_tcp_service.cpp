#include <implied_md_tcp_service.hpp>


implied_md_tcp_service::implied_md_tcp_service
(
	md::md_book& book,
	const std::string a_ip,
	unsigned short int a_port,
	const std::string b_ip,
	unsigned short int b_port,
	md::md_implied_book::implier_type bid_implie_type,
	md::md_implied_book::implier_type ask_implie_type,
	implier* bid_implier,
	implier* ask_implier
):
_a(a_ip, a_port),
_b(b_ip, b_port)
{
	auto idx = book.current_idx();
	book.add_implited_book(bid_implie_type, ask_implie_type, bid_implier, ask_implier);
	_a.set_on_order([&, idx](const matching::order& o)
	{
		book.handle_a(o, idx);
	});
	_b.set_on_order([&, idx](const matching::order& o)
	{
		book.handle_b(o, idx);
	});
}

void implied_md_tcp_service::run()
{
	_a.run();
	_b.run();
}
