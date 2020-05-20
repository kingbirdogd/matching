#include <iostream>
#include <cstdlib>
#include <matching_zmq_service.hpp>
#include <matching/implied_spread_in_bid.hpp>
#include <matching/implied_spread_in_ask.hpp>
#include <matching/implied_spread_a_out_bid.hpp>
#include <matching/implied_spread_a_out_ask.hpp>
#include <matching/implied_spread_b_out_bid.hpp>
#include <matching/implied_spread_b_out_ask.hpp>


int main(int iArgc, char** pszArgv)
{

	if (1 != iArgc)
	{
		std::cout << "usage: test_zmq_matching_server <port1[1,65535]> " << std::endl;
		return -1;
	}
	int iPort1 = std::atoi(pszArgv[1]);
	/*
	int iPort2 = std::atoi(pszArgv[2]);
	int iPort3 = std::atoi(pszArgv[2]); */
	if (iPort1 < 1 || iPort1 > 65535)
	{
		std::cout << "usage: test_zmq_matching_server <port1[1,65535]> " << std::endl;
		return -2;
	}
	/*
	if (iPort2 < 1 || iPort2 > 65535)
	{
		std::cout << "usage: test_tcp_matching_server <port1[1,65535]> <port2[1,65535]> <port3[1,65535]>" << std::endl;
		return -3;
	}
	if (iPort3 < 1 || iPort3 > 65535)
	{
		std::cout << "usage: test_tcp_matching_server <port1[1,65535]> <port2[1,65535]> <port3[1,65535]>" << std::endl;
		return -3;
	}*/
	auto sPort1 = static_cast<unsigned short int>(iPort1); /*
	auto sPort2 = static_cast<unsigned short int>(iPort2);
	auto sPort3 = static_cast<unsigned short int>(iPort3);
   */
	zmq::context_t ctx;
	matching_zmq_service s1(ctx);
	//matching_tcp_service s2(sPort2);
	//matching_tcp_service s3(sPort3);
	auto& March = s1.get_engine(); /*
	auto& June = s2.get_engine();
	auto& Spread = s3.get_engine();
	matching::implied_spread_in_bid spread_bid_implier(1, &March, &June);
	matching::implied_spread_in_ask spread_ask_implier(1, &March, &June);
	matching::implied_spread_in_bid a_bid_implier(1, &Spread, &June);
	matching::implied_spread_in_ask a_ask_implier(1, &Spread, &June);
	matching::implied_spread_in_bid b_bid_implier(1, &March, &Spread);
	matching::implied_spread_in_ask b_ask_implier(1, &March, &Spread);
	Spread.set_bid_implier(&spread_bid_implier);
	Spread.set_ask_implier(&spread_ask_implier);
	March.set_bid_implier(&a_bid_implier);
	March.set_ask_implier(&a_ask_implier);
	June.set_bid_implier(&b_bid_implier);
	June.set_ask_implier(&b_ask_implier); */
	while (true)
	{
		s1.run();
		//s2.run();
		//s3.run();
	}
	return 0;
}



