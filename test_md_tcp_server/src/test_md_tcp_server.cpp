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
	if (iArgc < 12 || iArgc > 14)
	{
		std::cerr << "usage test_md_tcp_server "
				"<outright_ip> "
				"<outright_port> "
				"<a_ip> "
				"<a_port> "
				"<b_ip> "
				"<b_port> "
				"<bid_implited_type> "
				"<ask_implited_type> "
				"<bid_impliter> "
				"<ask_impliter> "
				"<bind_port> "
				"[bind_ip] "
				"[mini_tick] "
				<< std::endl;
		return -2;
	}
	std::string outright_ip = pszArgv[1];
	unsigned short int outright_port = static_cast<unsigned short int>(std::stoi(pszArgv[2]));
	std::string a_ip = pszArgv[3];
	unsigned short int a_port = static_cast<unsigned short int>(std::stoi(pszArgv[4]));
	std::string b_ip = pszArgv[5];
	unsigned short int b_port = static_cast<unsigned short int>(std::stoi(pszArgv[6]));
	std::string str_bid_implie_type = pszArgv[7];
	md::md_book::implier_type bid_implie_type;
	std::string str_ask_implie_type = pszArgv[8];
	md::md_book::implier_type ask_implie_type;
	if (str_bid_implie_type == "a_bid_b_bid")
	{
		bid_implie_type = md::md_book::implier_type::a_bid_b_bid;
	}
	else if (str_bid_implie_type == "a_bid_b_ask")
	{
		bid_implie_type = md::md_book::implier_type::a_bid_b_ask;
	}
	else if (str_bid_implie_type == "a_ask_b_bid")
	{
		bid_implie_type = md::md_book::implier_type::a_ask_b_bid;
	}
	else if (str_bid_implie_type == "a_ask_b_ask")
	{
		bid_implie_type = md::md_book::implier_type::a_ask_b_ask;
	}
	else if (str_bid_implie_type == "a_none_b_none")
	{
		bid_implie_type = md::md_book::implier_type::a_none_b_none;
	}
	else
	{
		std::cerr << "bid_implie_type type not support:" << str_bid_implie_type << std::endl;
		return -2;
	}

	if (str_ask_implie_type == "a_bid_b_bid")
	{
		ask_implie_type = md::md_book::implier_type::a_bid_b_bid;
	}
	else if (str_ask_implie_type == "a_bid_b_ask")
	{
		ask_implie_type = md::md_book::implier_type::a_bid_b_ask;
	}
	else if (str_ask_implie_type == "a_ask_b_bid")
	{
		ask_implie_type = md::md_book::implier_type::a_ask_b_bid;
	}
	else if (str_ask_implie_type == "a_ask_b_ask")
	{
		ask_implie_type = md::md_book::implier_type::a_ask_b_ask;
	}
	else if (str_ask_implie_type == "a_none_b_none")
	{
		ask_implie_type = md::md_book::implier_type::a_none_b_none;
	}
	else
	{
		std::cerr << "str_ask_implie_type type not support:" << str_ask_implie_type << std::endl;
		return -3;
	}
	implier* bid_implier = nullptr;
	implier* ask_implier = nullptr;
	std::string str_bid_implier = pszArgv[9];
	std::string str_ask_implier = pszArgv[10];
	if (str_bid_implier == "add_bid_implier")
	{
		bid_implier = new add_bid_implier(0);
	}
	else if (str_bid_implier == "add_ask_implier")
	{
		bid_implier = new add_ask_implier(0);
	}
	else if (str_bid_implier == "minus_bid_implier")
	{
		bid_implier = new minus_bid_implier(0);
	}
	else if (str_bid_implier == "minus_ask_implier")
	{
		bid_implier = new minus_ask_implier(0);
	}
	else if (str_bid_implier == "repo_out_bid_implier")
	{
		bid_implier = new repo_out_bid_implier(0);
	}
	else if (str_bid_implier == "repo_out_ask_implier")
	{
		bid_implier = new repo_out_ask_implier(0);
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
		ask_implier = new add_bid_implier(0);
	}
	else if (str_ask_implier == "add_ask_implier")
	{
		ask_implier = new add_ask_implier(0);
	}
	else if (str_ask_implier == "minus_bid_implier")
	{
		ask_implier = new minus_bid_implier(0);
	}
	else if (str_ask_implier == "minus_ask_implier")
	{
		ask_implier = new minus_ask_implier(0);
	}
	else if (str_bid_implier == "repo_out_bid_implier")
	{
		ask_implier = new repo_out_bid_implier(0);
	}
	else if (str_bid_implier == "repo_out_ask_implier")
	{
		ask_implier = new repo_out_ask_implier(0);
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
	unsigned short int bind_port = static_cast<unsigned short int>(std::stoi(pszArgv[11]));
	std::string bind_ip = "";
	unsigned long long mini_tick = 1;
	if (iArgc > 12)
	{
		bind_ip = pszArgv[12];
	}
	if (iArgc > 13)
	{
		mini_tick = std::stoull(pszArgv[11]);
	}

	md_tcp_service srv
	(
			outright_ip,
			outright_port,
			a_ip,
			a_port,
			b_ip,
			b_port,
			bid_implie_type,
			ask_implie_type,
			bid_implier,
			ask_implier,
			bind_port,
			bind_ip,
			mini_tick
	);
	while (true)
		srv.run();
	return 0;
}





