#ifndef MD_INC_MD_BOOK_ITEM_HPP_
#define MD_INC_MD_BOOK_ITEM_HPP_

namespace md
{
	struct book_item
	{
	public:
		enum book_side : unsigned long long
		{
			none = 0,
			bid = 1,
			ask = 2
		};
	public:
		long long price;
		unsigned long long quantity;
		book_side side;
    unsigned long long market_id ;
		book_item():
			price(0),
			quantity(0),
			side(book_side::none),
			market_id(0)
		{
		}
		~book_item() = default;
	};
}

#endif /* MD_INC_MD_BOOK_ITEM_HPP_ */
