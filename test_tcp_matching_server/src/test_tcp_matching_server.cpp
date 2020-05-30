#include <iostream>
#include <cstdlib>
#include <matching_tcp_service.hpp>
#include <matching/implied_spread_in_bid.hpp>
#include <matching/implied_spread_in_ask.hpp>
#include <matching/implied_spread_a_out_bid.hpp>
#include <matching/implied_spread_a_out_ask.hpp>
#include <matching/implied_spread_b_out_bid.hpp>
#include <matching/implied_spread_b_out_ask.hpp>
#include <matching/implied_repo_out_bid.hpp>
#include <matching/implied_repo_out_ask.hpp>


int main(int iArgc, char** pszArgv)
{
	if (6 != iArgc)
	{
		std::cout << "usage: test_tcp_matching_server <port1[1,65535]> <port2[1,65535]> <port3[1,65535]> <port4[1,65535]> <port5[1,65535]>" << std::endl;
		return -1;
	}
	int iPort1 = std::atoi(pszArgv[1]);
	int iPort2 = std::atoi(pszArgv[2]);
	int iPort3 = std::atoi(pszArgv[3]);
	int iPort4 = std::atoi(pszArgv[4]);
	int iPort5 = std::atoi(pszArgv[5]);
	if (iPort1 < 1 || iPort1 > 65535)
	{
		std::cout << "usage: test_tcp_matching_server <port1[1,65535]> <port2[1,65535]> <port3[1,65535]> <port4[1,65535]> <port5[1,65535]>" << std::endl;
		return -2;
	}
	if (iPort2 < 1 || iPort2 > 65535)
	{
		std::cout << "usage: test_tcp_matching_server <port1[1,65535]> <port2[1,65535]> <port3[1,65535]> <port4[1,65535]> <port5[1,65535]>" << std::endl;
		return -3;
	}
	if (iPort3 < 1 || iPort3 > 65535)
	{
		std::cout << "usage: test_tcp_matching_server <port1[1,65535]> <port2[1,65535]> <port3[1,65535]> <port4[1,65535]> <port5[1,65535]>" << std::endl;
		return -3;
	}
	auto sPort1 = static_cast<unsigned short int>(iPort1);
	auto sPort2 = static_cast<unsigned short int>(iPort2);
	auto sPort3 = static_cast<unsigned short int>(iPort3);
	auto sPort4 = static_cast<unsigned short int>(iPort4);
	auto sPort5 = static_cast<unsigned short int>(iPort5);
	matching_tcp_service s1(sPort1);
	matching_tcp_service s2(sPort2);
	matching_tcp_service s3(sPort3);
	matching_tcp_service s4(sPort4);
	matching_tcp_service s5(sPort5);
	auto& March = s1.get_engine();
	auto& June = s2.get_engine();
	auto& Spread = s3.get_engine();
	auto& Spot = s4.get_engine();
	auto& Repo = s5.get_engine();
	matching::implied_spread_in_bid spread_bid_implier(1, &March, &June);
	matching::implied_spread_in_ask spread_ask_implier(1, &March, &June);
	matching::implied_spread_a_out_bid a_bid_implier(1, &Spread, &June);
	matching::implied_spread_a_out_ask a_ask_implier(1, &Spread, &June);
	matching::implied_spread_b_out_bid b_bid_implier(1, &March, &Spread);
	matching::implied_spread_b_out_ask b_ask_implier(1, &March, &Spread);
	matching::implied_repo_out_bid spot_bid_implier(1, &March, &Repo);
	matching::implied_repo_out_ask spot_ask_implier(1, &March, &Repo);
	Spread.set_bid_implier(&spread_bid_implier);
	Spread.set_ask_implier(&spread_ask_implier);
	March.set_bid_implier(&a_bid_implier);
	March.set_ask_implier(&a_ask_implier);
	June.set_bid_implier(&b_bid_implier);
	June.set_ask_implier(&b_ask_implier);
	Spot.set_bid_implier(&spot_bid_implier);
	Spot.set_ask_implier(&spot_ask_implier);
	while (true)
	{
		s1.run();
		s2.run();
		s3.run();
		s4.run();
		s5.run();
	}
	return 0;
}



