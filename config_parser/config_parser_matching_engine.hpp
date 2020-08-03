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

  static auto my_read_inst_config(const nl::json &j, std::unordered_map<std::string, matching::engine*> &book_map) {
    std::clog << j["tick_sz"] << " " <<  j["port"] << '\n';
    matching_tcp_service *s = new matching_tcp_service(static_cast<unsigned short int>(j["tick_sz"]), j["port"]);
    auto &book = s->get_engine();
    const auto [it, success] = book_map.insert({j["book_name"], &book});
    if (!success) {
      std::clog << j["book_name"] << " is defined more than once!!!" << '\n';
      exit(-1);
    }
    //unsigned long long bps = j["maker_fees"];
    //return [](){};
    auto res = [&book, s, &j](std::unordered_map<std::string, matching::engine*> &book_map){
      for (auto& implier: j["impliers"]) {
        //std::cout << implier << '\n';
        if (implier["bi_type"].get<std::string>().compare("a_bid_implier") &&
            implier["ai_type"].get<std::string>().compare("a_ask_implier")) {
          if (book_map.find(implier["bi_leg1"]) == book_map.end() || book_map.find(implier["ai_leg1"]) == book_map.end()) {
            std::clog << "bi_leg1 and ai_leg1 are not both defined prepoerly for " << j["book_name"] << '\n';
            exit(-1);
          }

          matching::implied_spread_a_out_bid a_bid_implier(implier["bi_priority"],book_map[implier["bi_leg1"]],book_map[implier["bi_leg2"]],implier["bi_maker_fees"]);
          matching::implied_spread_a_out_ask a_ask_implier(implier["ai_priority"],book_map[implier["ai_leg1"]],book_map[implier["ai_leg2"]],implier["ai_maker_fees"]);
          book.set_bid_implier(&a_bid_implier);
          book.set_ask_implier(&a_ask_implier);
          return s;
        }
        else if (implier["bi_type"].get<std::string>().compare("b_bid_implier") &&
            implier["ai_type"].get<std::string>().compare("b_ask_implier")) {
          if (book_map.find(implier["bi_leg1"]) == book_map.end() || book_map.find(implier["ai_leg1"]) == book_map.end()) {
            std::clog << "bi_leg1 and ai_leg1 are not both defined prepoerly for " << j["book_name"] << '\n';
            exit(-1);
          }

          matching::implied_spread_b_out_bid b_bid_implier(implier["bi_priority"],book_map[implier["bi_leg1"]],book_map[implier["bi_leg2"]],implier["bi_maker_fees"]);
          matching::implied_spread_b_out_ask b_ask_implier(implier["ai_priority"],book_map[implier["ai_leg1"]],book_map[implier["ai_leg2"]],implier["ai_maker_fees"]);
          book.set_bid_implier(&b_bid_implier);
          book.set_ask_implier(&b_ask_implier);
          return s;
        }
        else if (implier["bi_type"].get<std::string>().compare("in_bid_implier") &&
                 implier["ai_type"].get<std::string>().compare("in_ask_implier")) {
          if (book_map.find(implier["bi_leg1"]) == book_map.end() || book_map.find(implier["ai_leg1"]) == book_map.end()) {
            std::clog << "bi_leg1 and ai_leg1 are not both defined prepoerly for " << j["book_name"] << '\n';
            exit(-1);
          }

          matching::implied_spread_in_bid spread_bid_implier(implier["bi_priority"],book_map[implier["bi_leg1"]],book_map[implier["bi_leg2"]],implier["bi_maker_fees"]);
          matching::implied_spread_in_ask spread_ask_implier(implier["ai_priority"],book_map[implier["ai_leg1"]],book_map[implier["ai_leg2"]],implier["ai_maker_fees"]);
          book.set_bid_implier(&spread_bid_implier);
          book.set_ask_implier(&spread_bid_implier);
          return s;
        }
        else {
          std::clog << "bid_implier and ask_implier are not both defined properly for " << j["book_name"] << '\n';
        }
      }
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

    std::vector<decltype(std::function{my_read_inst_config})::result_type> books_to_be_set_impliers;
    //for (auto& e : j["instruments"]) {
      //std::clog << e << '\n';
      for (auto &inst : j["instruments"]) {
        std::cout << inst << '\n';
        books_to_be_set_impliers.push_back(my_read_inst_config(inst, book_map_));
      }
    //}

    std::vector<matching_tcp_service *> engines;
    for (auto &b: books_to_be_set_impliers) {
      engines.push_back(b(book_map_));
    }
    return engines;
  }


private:
  std::string config_fname_;
  std::unordered_map<std::string, matching::engine*> book_map_;
};

#endif //ENGINE_CONFIG_PARSER_MATCHING_ENGINE_HPP
