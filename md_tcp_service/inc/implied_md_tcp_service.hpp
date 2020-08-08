#ifndef MD_TCP_SERVICE_INC_IMPLIED_MD_TCP_SERVICE_HPP_
#define MD_TCP_SERVICE_INC_IMPLIED_MD_TCP_SERVICE_HPP_


#include <md/md_book.hpp>
#include <matching_tcp_client.hpp>

class implied_md_tcp_service
{
private:
	matching_tcp_client _a;
	matching_tcp_client _b;
public:
	implied_md_tcp_service
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
	);
	implied_md_tcp_service() = delete;
	implied_md_tcp_service(const implied_md_tcp_service&) = delete;
	implied_md_tcp_service(implied_md_tcp_service&& srv):
		_a(std::move(srv._a)),
		_b(std::move(srv._b))
	{
	}
	implied_md_tcp_service& operator= (const implied_md_tcp_service&) = delete;
	implied_md_tcp_service& operator= (implied_md_tcp_service&& srv)
	{
		_a = std::move(srv._a);
		_b = std::move(srv._b);
		return *this;
	}
	void run();
};



#endif /* MD_TCP_SERVICE_INC_IMPLIED_MD_TCP_SERVICE_HPP_ */
