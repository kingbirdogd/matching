#include <matching/engine.hpp>
#include <ctime>

using namespace matching;

std::atomic<unsigned long long> engine::_id(static_cast<unsigned long long>(time(nullptr)) * 10000000);
unsigned long long engine::_node_id(0);




