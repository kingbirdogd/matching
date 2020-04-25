#include <iostream>
#include <cstdlib>
#include <matching_tcp_service.hpp>


int main(int iArgc, char** pszArgv)
{
	if (2 != iArgc)
	{
		std::cout << "usage: test_tcp_matching_server <port[1,65535]>" << std::endl;
		return -1;
	}
	int iPort = std::atoi(pszArgv[1]);
	if (iPort < 1 || iPort > 65535)
	{
		std::cout << "usage: test_tcp_matching_server <port[1,65535]>" << std::endl;
		return -2;
	}
	auto sPort = static_cast<unsigned short int>(iPort);
	matching_tcp_service s(sPort);
	while (true)
	{
		s.run();
	}
	return 0;
}



