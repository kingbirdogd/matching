#ifndef MD_TCP_SERVICE_INC_MD_TCP_SERVICE_HPP_
#define MD_TCP_SERVICE_INC_MD_TCP_SERVICE_HPP_

#include <net/tcp_service.hpp>
#include <md/md_book.hpp>
#include <matching_tcp_client.hpp>

class md_tcp_service
{
private:
	matching_tcp_client _outright;
	matching_tcp_client _a;
	matching_tcp_client _b;
	md::md_book _book;
	net::tcp_service _s;
public:
	md_tcp_service() = delete;
	md_tcp_service(const md_tcp_service&) = delete;
	md_tcp_service(md_tcp_service&) = delete;
	md_tcp_service& operator= (const md_tcp_service&) = delete;
	md_tcp_service& operator= (md_tcp_service&) = delete;
	md_tcp_service
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
		const std::string bind_ip = "",
		unsigned long long mtick = 1
	);
	~md_tcp_service() = default;
	void run();
private:
	void _handle_item(const md::book_item& item);
};


#endif /* MD_TCP_SERVICE_INC_MD_TCP_SERVICE_HPP_ */
