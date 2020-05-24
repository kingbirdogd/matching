#ifndef MD_INC_MD_BASIC_BOOK_HPP_
#define MD_INC_MD_BASIC_BOOK_HPP_
#include <map>
#include <unordered_map>
#include <md/book_item.hpp>
#include <matching/order.hpp>

namespace md
{
	struct basic_book
	{
	public:
		using bid_book = std::map<long long, book_item, std::greater<long long>>;
		using ask_book = std::map<long long, book_item, std::less<long long>>;
		using order_book = std::unordered_map<unsigned long long, matching::order>;
	public:
		order_book orders;
		bid_book bid;
		ask_book ask;
		book_item add_bid_quantity(long long price, unsigned long long quantity);
		book_item reduce_bid_quantity(long long price, unsigned long long quantity);
		book_item add_ask_quantity(long long price, unsigned long long quantity);
		book_item reduce_ask_quantity(long long price, unsigned long long quantity);
		book_item handle_odr(const matching::order& odr);
	};
}



#endif /* MD_INC_MD_BASIC_BOOK_HPP_ */
