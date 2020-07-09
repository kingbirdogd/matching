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
		unsigned long long market_id;
		book_item():
			price(0),
			quantity(0),
			side(book_side::none),
			market_id(0)
		{
		}
		~book_item() = default;
		bool operator==(const book_item &rhs) {
		  return ((    price == rhs.price)
		      &&  (quantity  == rhs.quantity)
		      &&  (     side == rhs.side)
		      &&  (market_id == rhs.market_id));
		}
    bool operator!=(const book_item &rhs) {
      return !(operator==(rhs));
    }
	};
}

#endif /* MD_INC_MD_BOOK_ITEM_HPP_ */
