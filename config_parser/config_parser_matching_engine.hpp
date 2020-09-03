#ifndef ENGINE_CONFIG_PARSER_MATCHING_ENGINE_HPP
#define ENGINE_CONFIG_PARSER_MATCHING_ENGINE_HPP

#include <iostream>
#include <fstream>
#include <matching_tcp_service.hpp>
#include <matching/implied_spread_in_bid.hpp>
#include <matching/implied_spread_in_ask.hpp>
#include <matching/implied_spread_a_out_bid.hpp>
#include <matching/implied_spread_a_out_ask.hpp>
#include <matching/implied_spread_b_out_bid.hpp>
#include <matching/implied_spread_b_out_ask.hpp>
#include <matching/implied_repo_out_bid.hpp>
#include <matching/implied_repo_out_ask.hpp>
#include "json.hpp"

namespace nl = nlohmann;

class JsonConfigParser {
public:
  JsonConfigParser(const std::string &fname) : config_fname_(fname) { }

  static void assert_books_exist(const std::string& book_name,
                                 const nl::json &implier,
                                 const std::unordered_map<std::string, matching::engine*> &book_map)
  {
    if (book_map.find(implier["bi_leg1"]) == book_map.end() || book_map.find(implier["bi_leg2"]) == book_map.end() ||
        book_map.find(implier["ai_leg1"]) == book_map.end() || book_map.find(implier["ai_leg2"]) == book_map.end())
    {
      std::clog << "bi_leg1/2 (" << implier["bi_leg1"] << "/" << implier["bi_leg2"]
                << ") ai_leg1/2 (" << implier["ai_leg1"] << "/" << implier["ai_leg2"]
                << ") are not defined properly for " << book_name << '\n';
      exit(-1);
    }
  }

  static auto my_read_inst_config(const nl::json &j, std::unordered_map<std::string, matching::engine*> &book_map) {
    //std::clog << j["tick_sz"] << " " <<  j["port"] << '\n';
    std::clog << "Setting up book: " << j["book_name"] << '\n';
    unsigned long long scaled_tick_sz = j["tick_sz"].get<double >()*j["factor"].get<unsigned long long>();
    std::clog << "  Port:" << j["port"] << "  Tick Size:" << j["tick_sz"] << "  Scaled Tick Size:" << scaled_tick_sz << '\n';
    matching_tcp_service *s = new matching_tcp_service(scaled_tick_sz, static_cast<unsigned short int>(j["port"]));
    matching::engine &book = s->get_engine();
    const auto [it, success] = book_map.insert({j["book_name"], &book});
    if (!success) {
      std::clog << j["book_name"] << " is defined more than once!!!" << '\n';
      exit(-1);
    }
    //auto res = [&book, s, &j](std::unordered_map<std::string, matching::engine*> &book_map){
    auto res = [s, &j](std::unordered_map<std::string, matching::engine*> &book_map){
      std::vector<matching_tcp_service*> engines;
      auto &book = s->get_engine();
      std::clog << "Setting up impliers for book: " << j["book_name"] << '\n';
      if (j["impliers"].size() == 0)
        engines.push_back(s);

      for (auto& implier: j["impliers"]) {
        std::clog << "  implier: " << implier << '\n';
        assert_books_exist(j["book_name"], implier, book_map);

        if (implier["bi_type"].get<std::string>().compare("a_bid_implier") == 0 &&
            implier["ai_type"].get<std::string>().compare("a_ask_implier") == 0) {
          matching::implied_spread_a_out_bid *a_bid_implier =new matching::implied_spread_a_out_bid(implier["bi_priority"],book_map[implier["bi_leg1"]],book_map[implier["bi_leg2"]],implier["bi_maker_fees"]);
          matching::implied_spread_a_out_ask *a_ask_implier =new matching::implied_spread_a_out_ask(implier["ai_priority"],book_map[implier["ai_leg1"]],book_map[implier["ai_leg2"]],implier["ai_maker_fees"]);
          book.set_bid_implier(a_bid_implier);
          book.set_ask_implier(a_ask_implier);
          engines.push_back(s);
        }
        else if (implier["bi_type"].get<std::string>().compare("b_bid_implier") == 0 &&
                 implier["ai_type"].get<std::string>().compare("b_ask_implier") == 0) {
          matching::implied_spread_b_out_bid *b_bid_implier =new matching::implied_spread_b_out_bid(implier["bi_priority"],book_map[implier["bi_leg1"]],book_map[implier["bi_leg2"]],implier["bi_maker_fees"]);
          matching::implied_spread_b_out_ask *b_ask_implier =new matching::implied_spread_b_out_ask(implier["ai_priority"],book_map[implier["ai_leg1"]],book_map[implier["ai_leg2"]],implier["ai_maker_fees"]);
          book.set_bid_implier(b_bid_implier);
          book.set_ask_implier(b_ask_implier);
          engines.push_back(s);
        }
        else if (implier["bi_type"].get<std::string>().compare("in_bid_implier") == 0 &&
                 implier["ai_type"].get<std::string>().compare("in_ask_implier") == 0) {
          matching::implied_spread_in_bid *spread_bid_implier =new matching::implied_spread_in_bid(implier["bi_priority"],book_map[implier["bi_leg1"]],book_map[implier["bi_leg2"]],implier["bi_maker_fees"]);
          matching::implied_spread_in_ask *spread_ask_implier =new matching::implied_spread_in_ask(implier["ai_priority"],book_map[implier["ai_leg1"]],book_map[implier["ai_leg2"]],implier["ai_maker_fees"]);
          book.set_bid_implier(spread_bid_implier);
          book.set_ask_implier(spread_bid_implier);
          engines.push_back(s);
        }
        else if (implier["bi_type"].get<std::string>().compare("repo_out_bid") == 0 &&
                 implier["ai_type"].get<std::string>().compare("repo_out_ask") == 0) {
          matching::implied_repo_out_bid *spot_bid_implier =new matching::implied_repo_out_bid(implier["bi_priority"],book_map[implier["bi_leg1"]],book_map[implier["bi_leg2"]],implier["bi_maker_fees"],implier["bi_factor"]);
          matching::implied_repo_out_ask *spot_ask_implier =new matching::implied_repo_out_ask(implier["ai_priority"],book_map[implier["ai_leg1"]],book_map[implier["ai_leg2"]],implier["ai_maker_fees"],implier["ai_factor"]);
          book.set_bid_implier(spot_bid_implier);
          book.set_ask_implier(spot_ask_implier);
          engines.push_back(s);
        }
        else {
          std::clog << "bid_implier and ask_implier are not both defined properly for " << j["book_name"] << '\n';
        }
      }
      return engines;
    };
    std::clog << "Done " << j["book_name"] << '\n';
    return res;
    for (auto& [key, value]: j.items()) {
      std::cout << key << "=" << value << '\n';
    }
  }

  auto read_config() {
    std::ifstream i(config_fname_);
    nl::json j;
    i >> j;
    std::clog << "Setting up underlying: " << j["underlying"] << '\n';
    std::clog << "           node_id   : " << j["node_id"]    << '\n';
    unsigned long long node_id = strtoull(j["node_id"].get<std::string>().c_str(), NULL, 10);
    matching::engine::set_node_id(node_id);

    std::vector<decltype(std::function{my_read_inst_config})::result_type> books_to_be_set_impliers;
    for (auto &inst : j["instruments"]) {
      //std::clog << inst << '\n';
      books_to_be_set_impliers.push_back(my_read_inst_config(inst, book_map_));
    }

    std::vector<matching_tcp_service *> engines;
    for (auto &b: books_to_be_set_impliers) {
      auto es = b(book_map_);
      engines.insert(engines.end(), std::make_move_iterator(es.begin()), std::make_move_iterator(es.end()));
      //engines.push_back(b(book_map_));
    }
    return engines;
  }


private:
  std::string config_fname_;
  std::unordered_map<std::string, matching::engine*> book_map_;
};

#endif //ENGINE_CONFIG_PARSER_MATCHING_ENGINE_HPP
