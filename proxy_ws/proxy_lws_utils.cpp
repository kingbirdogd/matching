#include "proxy_lws_utils.hpp"

namespace proxy {
  void read_inst_config(std::string fname) {
    std::ifstream i(fname);
    njson j;
    i >> j; //std::cout << std::setw(4) << j << std::endl;

    for (auto& inst : j) {
      std::clog << "Reading instrument config: " << inst << '\n';
      asset_pair_t asset_pair(inst["base"].get<uint16_t>(), inst["counter"].get<uint16_t>());
      auto book_ptr = get_book(asset_pair, true);
      uint16_t qty = inst["min_order_qty"].get<uint16_t>();
      //std::clog << "[" << qty << "]\n";
      book_ptr->set_min_order_qty(qty);
    }
  }
}