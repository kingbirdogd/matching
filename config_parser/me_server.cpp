#include "config_parser_matching_engine.hpp"

int main(int argc, char** argv) {
  JsonConfigParser c(argv[1]);
  auto engines = c.read_config();
  while (true)
    for (auto &s : engines) {
      s->run();
    }
  return 0;
}