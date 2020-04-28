#ifndef MATCHING_ORDER_INC_MATCHING_IMPLIED_BASE_HPP_
#define MATCHING_ORDER_INC_MATCHING_IMPLIED_BASE_HPP_

namespace matching
{
	struct implied_order;
	class implied_base
	{
	public:
		implied_base() = default;
		virtual ~implied_base() = default;
		virtual void handle_outright(implied_order& o) = 0;
		virtual void handle_implied(implied_order& o) = 0;
	};
};



#endif /* MATCHING_ORDER_INC_MATCHING_IMPLIED_BASE_HPP_ */
