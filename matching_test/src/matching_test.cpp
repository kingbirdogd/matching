#include <matching/engine.hpp>

void handle_order(const matching::order&)
{
}

int main()
{
	matching::engine e(handle_order);
	matching::order o;
	e.handle(o);
	return 0;
}




