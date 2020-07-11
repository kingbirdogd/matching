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

  std::clog << pszArgv[0] << "\n";
  std::clog << "==== VERSION ====\n" << VERSION << "\n=================\n" << std::endl;
  std::clog << "==== SUBMODULE_VERSION ====\n" << SUBMODULE_VERSION << "\n===========================\n" << std::endl;

	matching::engine::set_node_id(1);
	if (15 != iArgc)
	{
		std::cout << "usage: test_tcp_matching_server <factor> <port1[1,65535]> <port2[1,65535]> <port3[1,65535]> <port4[1,65535]> <port5[1,65535]>" << std::endl;
		return -1;
	}
  unsigned long long factor = strtoull(pszArgv[1], NULL, 10);
	int iPort1 = std::atoi(pszArgv[2]);
	int iPort2 = std::atoi(pszArgv[3]);
	int iPort3 = std::atoi(pszArgv[4]);
	int iPort4 = std::atoi(pszArgv[5]);
	int iPort5 = std::atoi(pszArgv[6]);
  int iPort6 = std::atoi(pszArgv[7]);

	char *ptr;
  unsigned long long iTickSz1 = factor * strtod(pszArgv[8] , &ptr); printf("iTickSz: %llu\n", iTickSz1);
  unsigned long long iTickSz2 = factor * strtod(pszArgv[9] , &ptr); printf("iTickSz: %llu\n", iTickSz2);
  unsigned long long iTickSz3 = factor * strtod(pszArgv[10] ,&ptr); printf("iTickSz: %llu\n", iTickSz3);
  unsigned long long iTickSz4 = factor * strtod(pszArgv[11], &ptr); printf("iTickSz: %llu\n", iTickSz4);
  unsigned long long iTickSz5 = factor * strtod(pszArgv[12], &ptr); printf("iTickSz: %llu\n", iTickSz5);
  unsigned long long iTickSz6 = factor * strtod(pszArgv[13], &ptr); printf("iTickSz: %llu\n", iTickSz6);

  unsigned long long bps      =          strtoll(pszArgv[14], &ptr, 10); printf("maker fees: %llu bps\n"  , bps);
  fflush(stdout);

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
  auto sPort6 = static_cast<unsigned short int>(iPort6);
	matching_tcp_service s1(iTickSz1, sPort1);
	matching_tcp_service s2(iTickSz2, sPort2);
	matching_tcp_service s3(iTickSz3, sPort3);
	matching_tcp_service s4(iTickSz4, sPort4);
	matching_tcp_service s5(iTickSz5, sPort5);
  matching_tcp_service s6(iTickSz6, sPort6);
	auto& ABook  = s1.get_engine();
	auto& BBook  = s2.get_engine();
	auto& Spread = s3.get_engine();
	auto& Spot   = s4.get_engine();
	auto& Repo   = s5.get_engine();
  auto& Flex   = s6.get_engine();
	matching::implied_spread_in_bid spread_bid_implier(1, &ABook , &BBook , bps);
	matching::implied_spread_in_ask spread_ask_implier(1, &ABook , &BBook , bps);
	matching::implied_spread_a_out_bid a_bid_implier(  1, &Spread, &BBook , 0);
	matching::implied_spread_a_out_ask a_ask_implier(  1, &Spread, &BBook , 0);
	matching::implied_spread_b_out_bid b_bid_implier(  1, &ABook , &Spread, 0);
	matching::implied_spread_b_out_ask b_ask_implier(  1, &ABook , &Spread, 0);
	matching::implied_repo_out_bid spot_bid_implier(   1, &BBook , &Repo  , 0, factor);
	matching::implied_repo_out_ask spot_ask_implier(   1, &BBook , &Repo  , 0, factor);
	Spread.set_bid_implier(&spread_bid_implier);
	Spread.set_ask_implier(&spread_ask_implier);
	ABook.set_bid_implier(&a_bid_implier);
	ABook.set_ask_implier(&a_ask_implier);
	BBook.set_bid_implier(&b_bid_implier);
	BBook.set_ask_implier(&b_ask_implier);
	Spot.set_bid_implier(&spot_bid_implier);
	Spot.set_ask_implier(&spot_ask_implier);
	while (true)
	{
		s1.run();
		s2.run();
		s3.run();
		s4.run();
		s5.run();
    s6.run();
	}
	return 0;
}



