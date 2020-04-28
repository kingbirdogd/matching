#include <matching/implied_base.hpp>
#include <matching/engine.hpp>

using namespace matching;

void implied_base::_handle(engine& e, implied_order& o)
{
	o.implied_ptr = this;
	e._handle(o);
}




