#include <net/tcp_client.hpp>
#include <rapid_ring/ring_buffer_queue.hpp>
#include <vector>

using out_buffer = std::vector<char>;

struct odr_query_request
{
	unsigned long long account_id;
	unsigned long long market_id;
	unsigned long long request_id;
};

struct odr_query_response_header
{
	unsigned long long account_id;
	unsigned long long market_id;
	unsigned long long request_id;
	unsigned long long odr_cnt;
};

struct odr_query_tcp_request : odr_query_request
{
	net::tcp_client* client;
};

struct order_query_tcp_response
{
	net::tcp_client* client;
	out_buffer* buff_ptr;
};

using request_queue = rapid_ring::spsc_ring_buffer_queue<odr_query_tcp_request, 8192>;
using response_queue = rapid_ring::mpsc_ring_buffer_queue<order_query_tcp_response, 8192>;

int main(int, char**)
{
	return 0;
}




