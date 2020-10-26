#include "config_parser_matching_engine2.hpp"

int main(int argc, char** argv) {
  JsonConfigParser c(argv[1]);
  auto engines = c.read_config();

  for (auto &s : engines) {
    net::tcp_service& tcp =s->get_tcp_service();
    fprintf(stderr, "Running book on %s:%d\n", tcp._bind_addr.c_str(), tcp._bind_port);
  }

  while (true)
    for (auto s : engines) {
      s->run();
    }
  return 0;
}