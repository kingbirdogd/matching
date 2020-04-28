#ifndef MATCHING_ORDER_INC_MATCHING_IMPLIED_BASE_HPP_
#define MATCHING_ORDER_INC_MATCHING_IMPLIED_BASE_HPP_

namespace matching
{
	struct implied_order;
	class engine;
	class implied_base
	{
	private:
		friend engine;
	protected:
		static void _handle(engine& e, implied_order& o);
	public:
		implied_base() = default;
		virtual ~implied_base() = default;
		virtual void handle_outright(implied_order& o) = 0;
		virtual void handle_implied(implied_order& o) = 0;
	};
};



#endif /* MATCHING_ORDER_INC_MATCHING_IMPLIED_BASE_HPP_ */
