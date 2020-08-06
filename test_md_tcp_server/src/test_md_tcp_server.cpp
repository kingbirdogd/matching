#include <iostream>
#include <string>
#include <cstdlib>
#include <md_tcp_service.hpp>
#include <add_bid_implier.hpp>
#include <add_ask_implier.hpp>
#include <minus_bid_implier.hpp>
#include <minus_ask_implier.hpp>
#include <repo_out_bid_implier.hpp>
#include <repo_out_ask_implier.hpp>


int main(int iArgc, char** pszArgv)
{
	auto arg_cnt = iArgc  - 1;
	auto mode = arg_cnt % 8;
	auto implied_cnt = arg_cnt / 8;
	if (mode < 4 || mode > 7 || implied_cnt < 1)
	{
		std::cerr << "usage test_md_tcp_server "
				"<factor> "
				"<outright_ip> "
				"<outright_port> "
				"<bind_port> "
				"[bind_ip] "
				"[mini_tick] "
				"[maker_fees] "
				"<a_ip> "
				"<a_port> "
				"<b_ip> "
				"<b_port> "
				"<bid_implited_type> "
				"<ask_implited_type> "
				"<bid_impliter> "
				"<ask_impliter> "
				"<a_ip> "
				"<a_port> "
				"<b_ip> "
				"<b_port> "
				"<bid_implited_type> "
				"<ask_implited_type> "
				"<bid_impliter> "
				"<ask_impliter> "
				"..."
				<< std::endl;
		return -2;
	}
	unsigned long long factor = strtoull(pszArgv[1], NULL, 10);
	std::string outright_ip = pszArgv[2];
	unsigned short int outright_port = static_cast<unsigned short int>(std::stoi(pszArgv[3]));
	unsigned short int bind_port = static_cast<unsigned short int>(std::stoi(pszArgv[4]));
	std::string bind_ip = "";
	std::size_t idx = 5;
	if (mode >= 5)
	{
		bind_ip = pszArgv[5];
		idx = 6;
	}
	unsigned long long mini_tick = 1;
	unsigned long long bps       = 0;
	if (mode >= 6)
	{
		char *ptr;
		mini_tick = factor * strtod(pszArgv[6], &ptr);
		idx = 7;
	}
	if (mode >= 7)
	{
	    char *ptr;
	    bps = strtoll(pszArgv[7], &ptr, 10);
	    idx = 8;
	}
	md_tcp_service srv
	(
			outright_ip,
			outright_port,
			bind_port,
			bind_ip,
			mini_tick
	);
	for (int i = 0; i < implied_cnt; ++i)
	{
		std::string a_ip = pszArgv[idx];
		unsigned short int a_port = static_cast<unsigned short int>(std::stoi(pszArgv[idx + 1]));
		std::string b_ip = pszArgv[idx + 2];
		unsigned short int b_port = static_cast<unsigned short int>(std::stoi(pszArgv[idx + 3]));
		std::string str_bid_implie_type = pszArgv[idx + 4];
		md::md_implied_book::implier_type bid_implie_type;
		std::string str_ask_implie_type = pszArgv[idx + 5];
		md::md_implied_book::implier_type ask_implie_type;
		if (str_bid_implie_type == "a_bid_b_bid")
		{
			bid_implie_type = md::md_implied_book::implier_type::a_bid_b_bid;
		}
		else if (str_bid_implie_type == "a_bid_b_ask")
		{
			bid_implie_type = md::md_implied_book::implier_type::a_bid_b_ask;
		}
		else if (str_bid_implie_type == "a_ask_b_bid")
		{
			bid_implie_type = md::md_implied_book::implier_type::a_ask_b_bid;
		}
		else if (str_bid_implie_type == "a_ask_b_ask")
		{
			bid_implie_type = md::md_implied_book::implier_type::a_ask_b_ask;
		}
		else if (str_bid_implie_type == "a_none_b_none")
		{
			bid_implie_type = md::md_implied_book::implier_type::a_none_b_none;
		}
		else
		{
			std::cerr << "bid_implie_type type not support:" << str_bid_implie_type << std::endl;
			return -2;
		}

		if (str_ask_implie_type == "a_bid_b_bid")
		{
			ask_implie_type = md::md_implied_book::implier_type::a_bid_b_bid;
		}
		else if (str_ask_implie_type == "a_bid_b_ask")
		{
			ask_implie_type = md::md_implied_book::implier_type::a_bid_b_ask;
		}
		else if (str_ask_implie_type == "a_ask_b_bid")
		{
			ask_implie_type = md::md_implied_book::implier_type::a_ask_b_bid;
		}
		else if (str_ask_implie_type == "a_ask_b_ask")
		{
			ask_implie_type = md::md_implied_book::implier_type::a_ask_b_ask;
		}
		else if (str_ask_implie_type == "a_none_b_none")
		{
			ask_implie_type = md::md_implied_book::implier_type::a_none_b_none;
		}
		else
		{
			std::cerr << "str_ask_implie_type type not support:" << str_ask_implie_type << std::endl;
			return -3;
		}
		implier* bid_implier = nullptr;
		implier* ask_implier = nullptr;
		std::string str_bid_implier = pszArgv[idx + 6];
		std::string str_ask_implier = pszArgv[idx + 7];
		if (str_bid_implier == "add_bid_implier")
		{
			bid_implier = new add_bid_implier(bps);
		}
		else if (str_bid_implier == "add_ask_implier")
		{
			bid_implier = new add_ask_implier(bps);
		}
		else if (str_bid_implier == "minus_bid_implier")
		{
			bid_implier = new minus_bid_implier(bps);
		}
		else if (str_bid_implier == "minus_ask_implier")
		{
			bid_implier = new minus_ask_implier(bps);
		}
		else if (str_bid_implier == "repo_out_bid_implier")
		{
			bid_implier = new repo_out_bid_implier(bps, factor);
		}
		else if (str_bid_implier == "repo_out_ask_implier")
		{
			bid_implier = new repo_out_ask_implier(bps, factor);
		}
		else if (str_bid_implier == "none")
		{
			bid_implier = nullptr;
		}
		else
		{
			std::cerr << "bid_implier type not support:" << str_bid_implier << std::endl;
			return -3;
		}
		if (str_ask_implier == "add_bid_implier")
		{
			ask_implier = new add_bid_implier(bps);
		}
		else if (str_ask_implier == "add_ask_implier")
		{
			ask_implier = new add_ask_implier(bps);
		}
		else if (str_ask_implier == "minus_bid_implier")
		{
			ask_implier = new minus_bid_implier(bps);
		}
		else if (str_ask_implier == "minus_ask_implier")
		{
			ask_implier = new minus_ask_implier(bps);
		}
		else if (str_bid_implier == "repo_out_bid_implier")
		{
			ask_implier = new repo_out_bid_implier(bps, factor);
		}
		else if (str_bid_implier == "repo_out_ask_implier")
		{
			ask_implier = new repo_out_ask_implier(bps, factor);
		}
		else if (str_bid_implier == "none")
		{
			ask_implier = nullptr;
		}
		else
		{
			std::cerr << "ask_implier type not support:" << str_ask_implier << std::endl;
			return -3;
		}
		srv.add_implied_service(a_ip, a_port, b_ip, b_port, bid_implie_type, ask_implie_type, bid_implier, ask_implier);
		idx += 8;
	}
	while (true)
		srv.run();
	return 0;
}





