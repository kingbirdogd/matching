#include <matching/implied_base.hpp>
#include <matching/engine.hpp>

using namespace matching;

void implied_base::_place_order(engine* e, implied_order& o)
{
	o.implied_ptr = this;
	o.engine_type = order::order_engine_type::IMPLIED;
	e->_handle(o);
}




