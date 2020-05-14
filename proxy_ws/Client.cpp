#include <matching/order.hpp>
#include "Client.hpp"
#include "common/narrow.h"

extern std::unordered_map<unsigned long long, unsigned long long> client_to_engine_id_map;

std::ostream & operator << (std::ostream &os, const matching::order &o) {
  std::string type = "";
  std::string side = "";
  std::string status = "";
  std::string time_condition = "";
  std::string action = "";
  std::string matched_type = "";
  if (matching::order::order_type::LIMITED == o.type)
  {
    type = "LIMITED";
  }
  else
  {
    type = "MARKET";
  }
  if (matching::order::order_side::BUY == o.side)
  {
    side = "BUY";
  }
  else if (matching::order::order_side::SELL == o.side)
  {
    side = "SELL";
  }
  else if (matching::order::order_side::BUY_STOP == o.side)
  {
    side = "BUY_STOP";
  }
  else if (matching::order::order_side::SELL_STOP == o.side)
  {
    side = "SELL_STOP";
  }
  else if (matching::order::order_side::BUY_SELL_STOP == o.side)
  {
    side = "BUY_SELL_STOP";
  }
  else
  {
    side = "SELL_BUY_STOP";
  }
  if (o.order_state == matching::order::order_status_type::OPEN)
  {
    status = "OPEN";
  }
  else if (o.order_state == matching::order::order_status_type::PARTIAL_FILL)
  {
    status = "PARTIAL_FILL";
  }
  else if (o.order_state == matching::order::order_status_type::FILLED)
  {
    status = "FILLED";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED)
  {
    status = "CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MARKET_ORDER_NOTHING_MATCH)
  {
    status = "CANCELED_BY_MARKET_ORDER_NOTHING_MATCH";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_ALL_BY_IOC)
  {
    status = "CANCELED_ALL_BY_IOC";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_PARTIAL_BY_IOC)
  {
    status = "CANCELED_PARTIAL_BY_IOC";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_BY_FOK)
  {
    status = "CANCELED_BY_FOK";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MAKER_ONLY)
  {
    status = "CANCELED_BY_MAKER_ONLY";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_CANCEL_ORDER_ID_NOT_FOUND)
  {
    status = "REJECT_CANCEL_ORDER_ID_NOT_FOUND";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_AMEND_ORDER_ID_NOT_FOUND)
  {
    status = "REJECT_AMEND_ORDER_ID_NOT_FOUND";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY)
  {
    status = "REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_BUY_STOP_TRIGGER_LARGE_THAN_STOP_LIMITED)
  {
    status = "REJECT_BUY_STOP_TRIGGER_LESS_THAN_STOP_LIMITED";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED)
  {
    status = "REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_UNKNOW_ORDER_ACTION)
  {
    status = "REJECT_UNKNOW_ORDER_ACTION";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_QUANTITY_ZERO)
  {
    status = "REJECT_QUANTITY_ZERO";
  }
  else
  {
    status = "REJECT_LIMITE_ORDER_WITH_MARKET_PRICE";
  }
  if (matching::order::order_time_condition::GTC == o.time_condition)
  {
    time_condition = "GTC";
  }
  else if (matching::order::order_time_condition::IOC == o.time_condition)
  {
    time_condition = "IOC";
  }
  else if (matching::order::order_time_condition::FOK == o.time_condition)
  {
    time_condition = "FOK";
  }
  else if (matching::order::order_time_condition::MAKER_ONLY == o.time_condition)
  {
    time_condition = "MAKER_ONLY";
  }
  else
  {
    time_condition = "MAKER_ONLY_REPRICE";
  }
  if (matching::order::order_action_type::NEW == o.order_action)
  {
    action = "NEW";
  }
  else if (matching::order::order_action_type::CANCEL == o.order_action)
  {
    action = "CANCEL";
  }
  else
  {
    action = "AMEND";
  }
  if (matching::order::order_matched_type::TAKER == o.matched_type)
  {
    matched_type = "TAKER";
  }
  else
  {
    matched_type = "MAKER";
  }
  os
      << "action:" << action
      << ",side:" << side
      << ",time_condition:" << time_condition
      << ",order_id:" << o.order_id
      << ",client_order_id:" << o.client_order_id
      << ",quantity:" << o.quantity
      << ",display_quantity:" << o.display_quantity
      << ",remain_quantity:" << o.remain_quantity
      << ",price:" << o.price
      << ",buy_stop_trigger_price:" << o.buy_stop_trigger_price
      << ",buy_stop_limited_price:" << o.buy_stop_limited_price
      << ",sell_stop_trigger_price:" << o.sell_stop_trigger_price
      << ",sell_stop_limited_price:" << o.sell_stop_limited_price
      << ",last_match_price:" << o.last_match_price
      << ",last_match_quantity:" << o.last_match_quantity
      << ",last_matched_order_id:" << o.last_matched_order_id
      << ",last_matched_order_id2:" << o.last_matched_order_id2
      << ",matched_id:" << o.matched_id
      << ",status:" << status
      << ",matched_type:" << matched_type
      << std::endl;
  return os;
}
/*
json::Object handle_order(const matching::order& o)
{
  std::string type = "";
  std::string side = "";
  std::string status = "";
  std::string time_condition = "";
  std::string action = "";
  std::string matched_type = "";
  if (matching::order::order_type::LIMITED == o.type)
  {
    type = "LIMITED";
  }
  else
  {
    type = "MARKET";
  }
  if (matching::order::order_side::BUY == o.side)
  {
    side = "BUY";
  }
  else if (matching::order::order_side::SELL == o.side)
  {
    side = "SELL";
  }
  else if (matching::order::order_side::BUY_STOP == o.side)
  {
    side = "BUY_STOP";
  }
  else if (matching::order::order_side::SELL_STOP == o.side)
  {
    side = "SELL_STOP";
  }
  else if (matching::order::order_side::BUY_SELL_STOP == o.side)
  {
    side = "BUY_SELL_STOP";
  }
  else
  {
    side = "SELL_BUY_STOP";
  }
  if (o.order_state == matching::order::order_status_type::OPEN)
  {
    status = "OPEN";
  }
  else if (o.order_state == matching::order::order_status_type::PARTIAL_FILL)
  {
    status = "PARTIAL_FILL";
  }
  else if (o.order_state == matching::order::order_status_type::FILLED)
  {
    status = "FILLED";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED)
  {
    status = "CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MARKET_ORDER_NOTHING_MATCH)
  {
    status = "CANCELED_BY_MARKET_ORDER_NOTHING_MATCH";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_ALL_BY_IOC)
  {
    status = "CANCELED_ALL_BY_IOC";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_PARTIAL_BY_IOC)
  {
    status = "CANCELED_PARTIAL_BY_IOC";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_BY_FOK)
  {
    status = "CANCELED_BY_FOK";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_BY_MAKER_ONLY)
  {
    status = "CANCELED_BY_MAKER_ONLY";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_CANCEL_ORDER_ID_NOT_FOUND)
  {
    status = "REJECT_CANCEL_ORDER_ID_NOT_FOUND";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_AMEND_ORDER_ID_NOT_FOUND)
  {
    status = "REJECT_AMEND_ORDER_ID_NOT_FOUND";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY)
  {
    status = "REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_BUY_STOP_TRIGGER_LARGE_THAN_STOP_LIMITED)
  {
    status = "REJECT_BUY_STOP_TRIGGER_LESS_THAN_STOP_LIMITED";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED)
  {
    status = "REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMITED";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_UNKNOW_ORDER_ACTION)
  {
    status = "REJECT_UNKNOW_ORDER_ACTION";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_QUANTITY_ZERO)
  {
    status = "REJECT_QUANTITY_ZERO";
  }
  else
  {
    status = "REJECT_LIMITE_ORDER_WITH_MARKET_PRICE";
  }
  if (matching::order::order_time_condition::GTC == o.time_condition)
  {
    time_condition = "GTC";
  }
  else if (matching::order::order_time_condition::IOC == o.time_condition)
  {
    time_condition = "IOC";
  }
  else if (matching::order::order_time_condition::FOK == o.time_condition)
  {
    time_condition = "FOK";
  }
  else if (matching::order::order_time_condition::MAKER_ONLY == o.time_condition)
  {
    time_condition = "MAKER_ONLY";
  }
  else
  {
    time_condition = "MAKER_ONLY_REPRICE";
  }
  if (matching::order::order_action_type::NEW == o.order_action)
  {
    action = "NEW";
  }
  else if (matching::order::order_action_type::CANCEL == o.order_action)
  {
    action = "CANCEL";
  }
  else
  {
    action = "AMEND";
  }
  if (matching::order::order_matched_type::TAKER == o.matched_type)
  {
    matched_type = "TAKER";
  }
  else
  {
    matched_type = "MAKER";
  }
  client_to_engine_id_map[o.client_order_id] = o.order_id;
  elog.debug()
      << "action:" << action
      << ",side:" << side
      << ",time_condition:" << time_condition
      << ",order_id:" << o.order_id
      << ",client_order_id:" << o.client_order_id
      << ",quantity:" << o.quantity
      << ",display_quantity:" << o.display_quantity
      << ",remain_quantity:" << o.remain_quantity
      << ",price:" << o.price
      << ",buy_stop_trigger_price:" << o.buy_stop_trigger_price
      << ",buy_stop_limited_price:" << o.buy_stop_limited_price
      << ",sell_stop_trigger_price:" << o.sell_stop_trigger_price
      << ",sell_stop_limited_price:" << o.sell_stop_limited_price
      << ",last_match_price:" << o.last_match_price
      << ",last_match_quantity:" << o.last_match_quantity
      << ",last_matched_order_id:" << o.last_matched_order_id
      << ",last_matched_order_id2:" << o.last_matched_order_id2
      << ",matched_id:" << o.matched_id
      << ",status:" << status
      << ",matched_type:" << matched_type
      << std::endl;

  json::Object response;
  response.insert("action", json::String(action));
  response.insert("side", json::String(side));
  response.insert("time_condition", json::String(time_condition));
  response.insert("order_id", json::Integer(o.order_id));
  response.insert("client_order_id", json::Integer(o.client_order_id));
  response.insert("quantity", json::Integer(o.quantity));
  response.insert("display_quantity", json::Integer(o.display_quantity));
  response.insert("remain_quantity", json::Integer(o.remain_quantity));
  response.insert("price", json::Integer(o.price));
  response.insert("buy_stop_trigger_price", json::Integer(o.buy_stop_trigger_price));

  response.insert("buy_stop_limited_price", json::Integer(o.buy_stop_limited_price));
  response.insert("sell_stop_trigger_price", json::Integer(o.sell_stop_trigger_price));
  response.insert("sell_stop_limited_price", json::Integer(o.sell_stop_limited_price));
  response.insert("last_match_price", json::Integer(o.last_match_price));
  response.insert("last_match_quantity", json::Integer(o.last_match_quantity));
  response.insert("last_matched_order_id", json::Integer(o.last_matched_order_id));
  response.insert("last_matched_order_id2", json::Integer(o.last_matched_order_id2));
  response.insert("matched_id", json::Integer(o.matched_id));
  response.insert("status", json::String(status));
  response.insert("matched_type", json::String(matched_type));

  //response.insert("time",
  //    json::Integer(std::chrono::duration_cast<std::chrono::microseconds>(reply.time.time_since_epoch()).count()));

  return response;
}
*/

json::Object handle_order(const matching::order& o);

namespace proxy {
  std::shared_mutex Client::clients_rwlock;
  std::list<Client *> Client::all_clients;
  std::multimap<id_t, Client *> Client::users_clients;

  size_t Client::receive_response(const void *buf, size_t n) {
    std::lock_guard<std::mutex> callback_queue_lock(callback_queue_mutex);
    if (callback_queue.empty()) {
      throw std::ios_base::failure("received spurious response from core in Client::receive_response");
    }
    size_t reply_size = sizeof(matching::order);
    //auto reply_size = *static_cast<const core::Opcode *>(buf) == Success ? callback_queue.front()(buf, n) : sizeof(core::Opcode);
    if (n < reply_size) {
      return 0;
    }
    this->enqueue_work([this, callback = std::move(callback_queue.front()), msg = std::vector<uint8_t>(
        static_cast<const uint8_t *>(buf), static_cast<const uint8_t *>(buf) + reply_size)]() noexcept {
      try {
        try {
          callback(msg.data(), 0);
        }
        catch (const std::system_error &e) {
          if (elog.warn_enabled() && (e.code().category() != std::system_category() || e.code().value() != EPIPE)) {
            int i;
            //socket.ioctl(TIOCOUTQ, &i);
            elog.warn() << "dropping " << peer_addr << ": " << e.what() << " [" << i << ']' << std::endl;
          }
          throw;
        }
        catch (const std::exception &e) {
          if (elog.warn_enabled()) {
            int i;
            //socket.ioctl(TIOCOUTQ, &i);
            elog.warn() << "dropping " << peer_addr << ": " << e.what() << " [" << i << ']' << std::endl;
          }
          throw;
        }
      }
      catch (...) {
        try {
          //socket.shutdown(SHUT_RDWR);
        }
        catch (...) {
        }
      }
    });

    callback_queue.pop();
    if (throttled) {
      throttled = false;
      //selector->modify(socket, this, Selector::Flags::READABLE);
    }
    return reply_size;
  }

  void Client::message_received(const json::Value &msg) {
    const json::Object &request = msg.as_object();
    auto &request_method = *request.get("method").as_string();
    auto request_tag_ptr = request.find("tag");
    intmax_t tag = request_tag_ptr ? *request_tag_ptr->as_integer() : 0;
    if (request_method == "SetUserPublicKey") {
      if (user_id != 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else {
        struct SetUserPublicKey cmd;
        cmd.opcode = SetUserPublicKey;
        cmd.user_id = narrow_check<id_t>(*request.get("user_id").as_integer());
        auto &request_public_key = *request.get("public_key").as_array();
        if (request_public_key.size() != 2) {
          throw std::invalid_argument("invalid public key");
        }
        transcode<Base64Decoder>(cmd.public_key.x, sizeof cmd.public_key.x, *request_public_key[0]->as_string());
        transcode<Base64Decoder>(cmd.public_key.y, sizeof cmd.public_key.y, *request_public_key[1]->as_string());
        this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
          switch (*static_cast<const core::Opcode *>(buf)) {
            case Success: {
              if (n != 0) {
                return sizeof(struct SetUserPublicKeyReply);
              }
              auto &reply = *static_cast<const struct SetUserPublicKeyReply *>(buf);
              if (elog.debug_enabled()) {
                elog.debug() << ">> " << reply << std::endl;
              }
              json::Object response;
              if (tag) {
                response.insert("tag", json::Integer(tag));
              }
              response.insert("error_code", json::Integer(0));
              this->send_message(response);
              return 0;
            }
            case OutOfSequence:
              this->send_error(tag, 3, "User ID is out of sequence.");
              return 0;
            default:
              throw std::ios_base::failure("received illegal opcode");
          }
        });
      }
    } else if (request_method == "Authenticate") {
      if (auth_bucket.take(1) == 0) {
        this->send_error(tag, 6, "You are making authentication attempts too rapidly.");
      } else if (skip_auth_allowed) {
        id_t request_user_id = narrow_check<id_t>(*request.get("user_id").as_integer());
        this->switch_user(request_user_id);
        this->set_client_user_id(request_user_id);
        json::Object response;
        response.insert("error_code", json::Integer(0));
        this->send_message(response);
      } else {
        id_t request_user_id = narrow_check<id_t>(*request.get("user_id").as_integer());
        SHA1::digest_type request_cookie;
        if (transcode<Base64Decoder>(request_cookie.data(), request_cookie.size(), *request.get("cookie").as_string()) != request_cookie.size()) {
          throw std::invalid_argument("invalid cookie");
        }
        uint8_t client_nonce[16];
        if (transcode<Base64Decoder>(client_nonce, sizeof client_nonce, *request.get("nonce").as_string()) != sizeof client_nonce) {
          throw std::invalid_argument("invalid nonce");
        }
        auto &request_signature = *request.get("signature").as_array();
        if (request_signature.size() != 2) {
          throw std::invalid_argument("invalid signature");
        }
        auto signature_r = base64_to_mpn224(*request_signature[0]->as_string());
        auto signature_s = base64_to_mpn224(*request_signature[1]->as_string());
        core::PublicKey public_key;
        if (!get_user_public_key(request_user_id, public_key) || request_cookie != compute_cookie(request_user_id)) {
          this->send_error(tag, 7, "You sent an incorrect login cookie.");
        } else {
          std::array<mpn224_t, 3> pubkey = {{
                                                bytes_to_mpn224(public_key.x),
                                                bytes_to_mpn224(public_key.y),
                                                {{1, 0, 0, 0}}
                                            }};
          class SHA224 sha224;
          uint64_t user_id_be = htobe64(request_user_id);
          sha224.write(&user_id_be, sizeof user_id_be);
          sha224.write(nonce, sizeof nonce);
          sha224.write(client_nonce, sizeof client_nonce);
          if (ecp_verify(secp224k1_p, secp224k1_a, *secp224k1_G, secp224k1_n, pubkey[0].data(), bytes_to_mpn224(*reinterpret_cast<const uint8_t (*)[SHA224::digest_size]>(sha224.digest().data())).data(), signature_r.data(), signature_s.data(), 4)) {
            this->switch_user(request_user_id);
            this->set_client_user_id(request_user_id);
            json::Object response;
            if (tag) {
              response.insert("tag", json::Integer(tag));
            }
            response.insert("error_code", json::Integer(0));
            this->send_message(response);
          } else {
            this->send_error(tag, 7, "You sent an incorrect signature. This probably means you used a wrong passphrase.");
          }
        }
      }
    } else if (request_method == "GetBalances") {
      if (info_bucket.take(1) == 0) {
        this->send_error(tag, 6, "You are making information requests too rapidly.");
      } else if (~user_id == 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else {
        struct GetBalances cmd;
        cmd.opcode = GetBalances;
        cmd.user_id = user_id;
        this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
          switch (*static_cast<const core::Opcode *>(buf)) {
            case Success: {
              auto &reply = *static_cast<const struct GetBalancesReply *>(buf);
              if (n != 0) {
                return sizeof(struct GetBalancesReply) + (n < sizeof(struct GetBalancesReply) ? 0 : reply.balances_count * sizeof(struct BalanceInfo));
              }
              if (elog.debug_enabled()) {
                elog.debug() << ">> " << reply << std::endl;
              }
              json::Object response;
              if (tag) {
                response.insert("tag", json::Integer(tag));
              }
              response.insert("error_code", json::Integer(0));
              json::Array response_balances;
              response_balances->reserve(reply.balances_count);
              for (size_t i = 0; i < reply.balances_count; ++i) {
                auto &balance = reply.balances[i];
                json::Object response_balance;
                response_balance.insert("asset", json::Integer(balance.asset_id));
                response_balance.insert("balance", json::Integer(balance.balance));
                response_balance.insert("reserved_balance", json::Integer(balance.reserved_balance));
                response_balance.insert("total_balance", json::Integer(balance.total_balance));
                response_balances.insert(std::move(response_balance));
              }
              response.insert("balances", std::move(response_balances));
              this->send_message(response);
              return 0;
            }
            default:
              throw std::ios_base::failure("received illegal opcode");
          }
        });
      }
    } else if (request_method == "GetBalance") {
      if (~user_id == 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else {
        id_t user_id = narrow_check<id_t>(*request.get("user_id").as_integer());
        asset_t asset_id = narrow_check<asset_t>(*request.get("asset").as_integer());
        this->do_privileged(tag, asset_id, [this, tag, user_id, asset_id]() {
          struct GetBalance cmd;
          cmd.opcode = GetBalance;
          cmd.user_id = user_id;
          cmd.asset_id = asset_id;
          this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
            switch (*static_cast<const core::Opcode *>(buf)) {
              case Success: {
                if (n != 0) {
                  return sizeof(struct GetBalanceReply);
                }
                auto &reply = *static_cast<const struct GetBalanceReply *>(buf);
                if (elog.debug_enabled()) {
                  elog.debug() << ">> " << reply << std::endl;
                }
                json::Object response;
                if (tag) {
                  response.insert("tag", json::Integer(tag));
                }
                response.insert("error_code", json::Integer(0));
                response.insert("balance", json::Integer(reply.balance));
                response.insert("seq", json::Integer(reply.seq_num));
                this->send_message(response);
                return 0;
              }
              case NotFound:
                this->send_error(tag, 1, "There is no such user.");
                return 0;
              default:
                throw std::ios_base::failure("received illegal opcode");
            }
          });
        });
      }
    } else if (request_method == "AdjustBalance") {
      if (~user_id == 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else {
        id_t user_id = narrow_check<id_t>(*request.get("user_id").as_integer());
        asset_t asset_id = narrow_check<asset_t>(*request.get("asset").as_integer());
        int64_t delta = narrow_check<int64_t>(*request.get("delta").as_integer());
        uint64_t seq_num = narrow_check<uint64_t>(*request.get("seq").as_integer());
        this->do_privileged(tag, asset_id, [this, tag, user_id, asset_id, delta, seq_num]() {
          struct AdjustBalance cmd;
          cmd.opcode = AdjustBalance;
          cmd.user_id = user_id;
          cmd.asset_id = asset_id;
          cmd.delta = delta;
          cmd.seq_num = seq_num;
          this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
            switch (*static_cast<const core::Opcode *>(buf)) {
              case Success: {
                if (n != 0) {
                  return sizeof(struct AdjustBalanceReply);
                }
                auto &reply = *static_cast<const struct AdjustBalanceReply *>(buf);
                if (elog.debug_enabled()) {
                  elog.debug() << ">> " << reply << std::endl;
                }
                json::Object response;
                if (tag) {
                  response.insert("tag", json::Integer(tag));
                }
                response.insert("error_code", json::Integer(0));
                response.insert("balance", json::Integer(reply.balance));
                this->send_message(response);
                return 0;
              }
              case NotFound:
                this->send_error(tag, 1, "There is no such user.");
                return 0;
              case OutOfSequence:
                this->send_error(tag, 3, "Bad sequence number.");
                return 0;
              case InsufficientFunds:
                this->send_error(tag, 4, "There are insufficient funds in the specified account.");
                return 0;
              default:
                throw std::ios_base::failure("received illegal opcode");
            }
          });
        });
      }
    } else if (request_method == "GetOrders") {
      if (info_bucket.take(1) == 0) {
        this->send_error(tag, 6, "You are making information requests too rapidly.");
      } else if (~user_id == 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else {
        struct GetOrders cmd;
        cmd.opcode = GetOrders;
        cmd.user_id = user_id;
        this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
          switch (*static_cast<const core::Opcode *>(buf)) {
            case Success: {
              auto &reply = *static_cast<const struct GetOrdersReply *>(buf);
              if (n != 0) {
                return sizeof(struct GetOrdersReply) + (n < sizeof(struct GetOrdersReply) ? 0 : reply.orders_count * sizeof(struct Order));
              }
              if (elog.debug_enabled()) {
                elog.debug() << ">> " << reply << std::endl;
              }
              json::Object response;
              if (tag) {
                response.insert("tag", json::Integer(tag));
              }
              response.insert("error_code", json::Integer(0));
              json::Array response_orders;
              response_orders->reserve(reply.orders_count);
              for (size_t i = 0; i < reply.orders_count; ++i) {
                response_orders.insert(order_to_json(reply.orders[i]));
              }
              response.insert("orders", std::move(response_orders));
              this->send_message(response);
              return 0;
            }
            default:
              throw std::ios_base::failure("received illegal opcode");
          }
        });
      }
    } else if (request_method == "EstimateMarketOrder") {
      if (order_bucket.take(1) == 0) {
        this->send_error(tag, 6, "You are sending orders too rapidly.");
      } else {
        asset_pair_t asset_pair(narrow_check<asset_t>(*request.get("base").as_integer()), narrow_check<asset_t>(*request.get("counter").as_integer()));
        auto book_ptr = get_book(asset_pair);
        auto request_quantity_ptr = request.find("quantity");
        auto request_total_ptr = request.find("total");
        if (!book_ptr) {
          this->send_error(tag, 1, "You specified an invalid asset pair.");
        } else if (!request_quantity_ptr == !request_total_ptr) {
          this->send_error(tag, 8, "You must specify either quantity or total for a market order.");
        } else {
          std::pair<uint64_t, uint64_t> result(~uint64_t(), ~uint64_t());
          if (request_quantity_ptr) {
            auto quantity = narrow_check<decltype(OrderParams::quantity)>(*request_quantity_ptr->as_integer());
            if (quantity == 0) {
              this->send_error(tag, 8, "Quantity must not be zero.");
            } else {
              result = book_ptr->estimate_market_order_from_quantity(quantity);
            }
          } else { // (request_total_ptr)
            auto total = narrow_check<decltype(OrderParams::quantity)>(*request_total_ptr->as_integer());
            if (total == 0) {
              this->send_error(tag, 8, "Total must not be zero.");
            } else {
              result = book_ptr->estimate_market_order_from_total(total);
            }
          }
          if (~result.first || ~result.second) {
            json::Object response;
            if (tag) {
              response.insert("tag", json::Integer(tag));
            }
            response.insert("error_code", json::Integer(0));
            response.insert("quantity", json::Integer(result.first));
            response.insert("total", json::Integer(result.second));
            this->send_message(response);
          }
        }
      }
    } else if (request_method == "PlaceOrder") {
//      if (request_tonce_ptr && (tonce = narrow_check<uint64_t>(*request_tonce_ptr->as_integer())) == 0) {
//        this->send_error(tag, 8, "Tonce must not be zero.");
//      }
      auto px_ptr                   = request.find("price");
      auto qty_ptr                  = request.find("quantity");
      auto client_order_id_ptr      = request.find("client_order_id");
      auto order_id_ptr             = request.find("order_id");
      auto display_qty_ptr          = request.find("display_quantity");
      auto buy_stop_trigger_px_ptr  = request.find("buy_stop_trigger_price");
      auto buy_stop_limited_px_ptr  = request.find("buy_stop_limited_price");
      auto sell_stop_trigger_px_ptr = request.find("sell_stop_trigger_price");
      auto sell_stop_limited_px_ptr = request.find("sell_stop_limited_price");

      auto side_ptr           = request.find("side");
      auto order_action_ptr   = request.find("order_action");
      auto time_condition_ptr = request.find("time_condition");

      auto px                   = px_ptr                   ? narrow_check<decltype(OrderParams::price   )>(*px_ptr->as_integer()) : 0;
      auto qty                  = qty_ptr                  ? narrow_check<decltype(OrderParams::quantity)>(*qty_ptr->as_integer()) : 0;
      auto client_order_id      = client_order_id_ptr      ? narrow_check<uint64_t>(*client_order_id_ptr->as_integer()) :0;
      auto order_id             = order_id_ptr             ? narrow_check<uint64_t>(*order_id_ptr->as_integer()) :0;
      auto display_qty          = display_qty_ptr          ? narrow_check<uint64_t>(*display_qty_ptr->as_integer()):0;
      auto buy_stop_trigger_px  = buy_stop_trigger_px_ptr  ? narrow_check<uint64_t>(*buy_stop_trigger_px_ptr->as_integer()):0;
      auto buy_stop_limited_px  = buy_stop_limited_px_ptr  ? narrow_check<uint64_t>(*buy_stop_limited_px_ptr->as_integer()):0;
      auto sell_stop_trigger_px = sell_stop_trigger_px_ptr ? narrow_check<uint64_t>(*sell_stop_trigger_px_ptr->as_integer()):0;
      auto sell_stop_limited_px = sell_stop_limited_px_ptr ? narrow_check<uint64_t>(*sell_stop_limited_px_ptr->as_integer()):0;

      auto side           = side_ptr           ? *dynamic_cast<const json::String *>(side_ptr          )->as_string() : "";
      auto order_action   = order_action_ptr   ? *dynamic_cast<const json::String *>(order_action_ptr  )->as_string() : "";
      auto time_condition = time_condition_ptr ? *dynamic_cast<const json::String *>(time_condition_ptr)->as_string() : "";

      matching::order o;
      o.order_action =  order_action.compare("NEW")    == 0 ? matching::order::NEW    :
                       (order_action.compare("CANCEL") == 0 ? matching::order::CANCEL :
                                                                 matching::order::AMEND  );
      o.side =  side.compare("SELL"     ) == 0 ? matching::order::order_side::SELL     :
               (side.compare("BUY"      ) == 0 ? matching::order::order_side::BUY      :
               (side.compare("BUY_STOP" ) == 0 ? matching::order::order_side::BUY_STOP :
                                                    matching::order::order_side::SELL_STOP ));
      o.time_condition =  time_condition.compare("FOK"               ) == 0 ? matching::order::FOK :
                         (time_condition.compare("IOC"               ) == 0 ? matching::order::IOC :
                         (time_condition.compare("MAKER_ONLY_REPRICE") == 0 ? matching::order::MAKER_ONLY_REPRICE :
                         (time_condition.compare("MAKER_ONLY"        ) == 0 ? matching::order::MAKER_ONLY :
                                                                                  matching::order::GTC)));
      o.price                   = px;
      o.quantity                = qty;
      o.client_order_id         = client_order_id;
      o.order_id                = order_id;
      o.display_quantity        = qty;   // TODO: use display_qty
      o.buy_stop_trigger_price  = buy_stop_trigger_px;
      o.buy_stop_limited_price  = buy_stop_limited_px;
      o.sell_stop_trigger_price = sell_stop_trigger_px;
      o.sell_stop_limited_price = sell_stop_limited_px;

      bool transient = false;
      auto callback = [this, tag, transient](const void *buf, size_t n) -> size_t {
        auto &reply = (*static_cast<const matching::order *>(buf));
        json::Object response = handle_order(reply);
        if (tag)
          response.insert("tag", json::Integer(tag));
        //this->send_message(response);
      };
      this->do_request(tag, o, std::move(callback));

/*
      if (order_bucket.take(1) == 0) {
        this->send_error(tag, 6, "You are sending orders too rapidly.");
      } else if (~user_id == 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else {
        auto request_tonce_ptr = request.find("tonce");
        uint64_t tonce = 0;
        asset_pair_t asset_pair(narrow_check<asset_t>(*request.get("base").as_integer()), narrow_check<asset_t>(*request.get("counter").as_integer()));
        auto request_price_ptr = request.find("price");
        if (request_tonce_ptr && (tonce = narrow_check<uint64_t>(*request_tonce_ptr->as_integer())) == 0) {
          this->send_error(tag, 8, "Tonce must not be zero.");
        } else if (request_price_ptr) {
          struct PlaceOrderEx cmd;
          cmd.order_params.quantity = narrow_check<decltype(OrderParams::quantity)>(*request.get("quantity").as_integer());

          if ((cmd.order_params.price = narrow_check<decltype(OrderParams::price)>(*request_price_ptr->as_integer())) == 0) {
            this->send_error(tag, 8, "Price must not be zero.");
          } else if (!check_muladddiv(static_cast<ulong>(::labs(cmd.order_params.quantity)), cmd.order_params.price, PRICE_SCALE - 1, PRICE_SCALE)) {
            this->send_error(tag, 8, "Order total would overflow.");
          } else {
            cmd.user_id = user_id;
            cmd.order_params.tonce = tonce;
            cmd.order_params.asset_pair = asset_pair;
            cmd.order_params.flags = OrderFlags{};
            cmd.flags = PlaceOrderEx::Flags{};

            uint16_t min_order_qty = get_book(asset_pair, true)->get_min_order_qty();
            if (labs(cmd.order_params.quantity) < min_order_qty) {
              this->send_error(tag, 8, "Order quantity must be greater than minimum required order quantity.");
            } else {
              bool transient = false;
              auto request_persist_ptr = request.find("persist");
              if (request_persist_ptr) {
                if (auto boolean_ptr = dynamic_cast<const json::Boolean *>(request_persist_ptr)) {
                  transient = !**boolean_ptr;
                } else if (auto string_ptr = dynamic_cast<const json::String *>(request_persist_ptr)) {
                  if (uplink->protocol_version() < 1 || **string_ptr != "fill_or_kill") {
                    throw std::invalid_argument("invalid persist flag");
                  }
                  cmd.flags |= PlaceOrderEx::FILL_OR_KILL;
                } else {
                  throw std::invalid_argument("invalid persist flag");
                }
              }
              cmd.order_params.flags.fee_external = true;
              auto request_fee_ptr = request.find("fee");
              if (proxy::fee_control_allowed && request_fee_ptr) {
                auto &request_fee = *request_fee_ptr->as_string();
                if (request_fee == "ext") {
                  cmd.order_params.flags.fee_external = true;
                } else if (request_fee == "int") {
                  cmd.order_params.flags.fee_external = false;
                } else {
                  throw std::invalid_argument("invalid fee flag");
                }
              }
              if (auto request_origin_ptr = request.find("origin")) {
                auto &request_origin = *request_origin_ptr->as_string();
                if (request_origin == "mobile") {
                  cmd.order_params.flags.origin = static_cast<std::underlying_type_t<OrderOrigin>>(OrderOrigin::MOBILE);
                } else if (request_origin == "bracket") {
                  cmd.order_params.flags.origin = static_cast<std::underlying_type_t<OrderOrigin>>(OrderOrigin::BRACKET);
                } else {
                  elog.warn() << ">> " << "invalid origin" << std::endl;
                }
              }
              auto callback = [this, tag, transient](const void *buf, size_t n) -> size_t {
                switch (*static_cast<const core::Opcode *>(buf)) {
                  case Success: {
                    if (n != 0) {
                      return sizeof(struct PlaceOrderReply);
                    }
                    auto &reply = *static_cast<const struct PlaceOrderReply *>(buf);
                    if (elog.trace_enabled()) {
                      std::thread::id this_id = std::this_thread::get_id();
                      elog.trace() << "Thread(" << this_id << ") >> " << reply << std::endl;
                    } else if (elog.debug_enabled()) {
                      elog.debug() << " >> " << reply << std::endl;
                    }

                    if (transient) {
                      std::lock_guard<std::mutex> transient_orders_lock(transient_orders_mutex);
                      transient_orders.insert(reply.order_id);
                    }
                    json::Object response;
                    if (tag) {
                      response.insert("tag", json::Integer(tag));
                    }
                    response.insert("error_code", json::Integer(0));
                    response.insert("id", json::Integer(reply.order_id));
                    response.insert("time", json::Integer(std::chrono::duration_cast<std::chrono::microseconds>(reply.time.time_since_epoch()).count()));
                    this->send_message(response);
                    return 0;
                  }
                  case NotFound:
                    this->send_error(tag, 1, "You specified an invalid asset pair.");
                    return 0;
                  case OutOfSequence:
                    this->send_error(tag, 3, "Tonce already in use.");
                    return 0;
                  case InsufficientFunds:
                    this->send_error(tag, 4, "You have insufficient funds.");
                    return 0;
                  case TooMany:
                    this->send_error(tag, 5, "You have too many outstanding orders.");
                    return 0;
                  case TooFast:
                    this->send_error(tag, 6, "You are sending orders too rapidly.");
                    return 0;
                  case InvalidArgument:
                    // can happen if the trade engine rounds the requested price down to zero
                    this->send_error(tag, 8, "Price must not be zero.");
                    return 0;
                  case NotAllowed:
                    this->send_error(tag, 9, "Post-only order with these parameters would result in an immediate match.");
                    return 0;
                  default:
                    throw std::ios_base::failure("received illegal opcode");
                }
              };
              parse_post_only_flag(request, cmd);
              if (cmd.flags) {
                cmd.opcode = core::PlaceOrderEx;
                this->do_request(tag, cmd, std::move(callback));
              } else {
                cmd.opcode = PlaceOrder;
                this->do_request(tag, static_cast<const struct PlaceOrder &>(cmd), std::move(callback));
              }
            }
          }
        } else {
          struct ExecuteMarketOrder cmd;
          auto request_quantity_ptr = request.find("quantity");
          auto request_total_ptr = request.find("total");
          cmd.flags = OrderFlags{};
          if (request_quantity_ptr) {
            cmd.quantity = narrow_check<decltype(OrderParams::quantity)>(*request_quantity_ptr->as_integer());
            cmd.opcode = ExecuteBaseMarketOrder;
          } else if (request_total_ptr) {
            cmd.quantity = narrow_check<decltype(OrderParams::quantity)>(*request_total_ptr->as_integer());
            cmd.opcode = ExecuteCounterMarketOrder;
          }
          if (!request_quantity_ptr == !request_total_ptr) {
            this->send_error(tag, 8, "You must specify either quantity or total for a market order.");
          } else if (cmd.quantity == 0) {
            this->send_error(tag, 8, "Quantity must not be zero.");
          } else {
            cmd.user_id = user_id;
            cmd.tonce = tonce;
            cmd.asset_pair = asset_pair;
            if (auto request_origin_ptr = request.find("origin")) {
              auto &request_origin = *request_origin_ptr->as_string();
              if (request_origin == "mobile") {
                cmd.flags.origin = static_cast<std::underlying_type_t<OrderOrigin>>(OrderOrigin::MOBILE);
              } else {
                throw std::invalid_argument("invalid origin");
              }
            }
            this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
              switch (*static_cast<const core::Opcode *>(buf)) {
                case Success: {
                  if (n != 0) {
                    return sizeof(struct ExecuteMarketOrderReply);
                  }
                  auto &reply = *static_cast<const struct ExecuteMarketOrderReply *>(buf);
                  if (elog.debug_enabled()) {
                    elog.debug() << ">> " << reply << std::endl;
                  }
                  json::Object response;
                  if (tag) {
                    response.insert("tag", json::Integer(tag));
                  }
                  response.insert("error_code", json::Integer(0));
                  response.insert("remaining", json::Integer(reply.remaining));
                  this->send_message(response);
                  return 0;
                }
                case NotFound:
                  this->send_error(tag, 1, "You specified an invalid asset pair.");
                  return 0;
                case OutOfSequence:
                  this->send_error(tag, 3, "Tonce already in use.");
                  return 0;
                case TooFast:
                  this->send_error(tag, 6, "You are sending orders too rapidly.");
                  return 0;
                default:
                  throw std::ios_base::failure("received illegal opcode");
              }
            });
          }
        }
      } */
    } else if (request_method == "ModifyOrder" && api_version >= 1) {
      if (uplink->protocol_version() < 6) {
        this->send_error(tag, 8, "unknown method");
      } else if (order_bucket.take(1) == 0) {
        this->send_error(tag, 6, "You are sending orders too rapidly.");
      } else if (~user_id == 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else {
        struct ModifyOrder cmd;
        auto request_order_id_ptr = request.find("id");
        auto request_tonce_ptr = request.find("tonce");
        auto request_quantity_delta_ptr = request.find("quantity_delta");
        auto request_price_ptr = request.find("price");
        if (request_order_id_ptr) {
          cmd.order_id = narrow_check<id_t>(*request_order_id_ptr->as_integer());
          cmd.opcode = ModifyOrder;
        } else if (request_tonce_ptr) {
          cmd.tonce = narrow_check<uint64_t>(*request_tonce_ptr->as_integer());
          cmd.opcode = ModifyOrderByTonce;
        }
        if (!request_order_id_ptr == !request_tonce_ptr) {
          this->send_error(tag, 8, "You must specify either order ID or tonce.");
        } else if (request_tonce_ptr && !cmd.tonce) {
          this->send_error(tag, 8, "Tonce must not be zero.");
        } else if (!request_quantity_delta_ptr && !request_price_ptr) {
          this->send_error(tag, 8, "You must specify quantity delta and/or price.");
        } else if (request_quantity_delta_ptr ? (cmd.quantity_delta = narrow_check<int64_t>(*request_quantity_delta_ptr->as_integer())) == 0 : (cmd.quantity_delta = 0, false)) {
          this->send_error(tag, 8, "Quantity delta must not be zero.");
        } else if (request_price_ptr ? (cmd.price = narrow_check<uint64_t>(*request_price_ptr->as_integer())) == 0 : (cmd.price = 0, false)) {
          this->send_error(tag, 8, "Price must not be zero.");
        } else {
          cmd.user_id = user_id;
          cmd.flags = {};
          parse_post_only_flag(request, cmd);
          this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
            switch (*static_cast<const core::Opcode *>(buf)) {
              case Success: {
                if (n != 0) {
                  return sizeof(struct ModifyOrderReply);
                }
                auto &reply = *static_cast<const struct ModifyOrderReply *>(buf);
                if (elog.debug_enabled()) {
                  elog.debug() << ">> " << reply << std::endl;
                }
                json::Object response = order_to_json(reply.order);
                if (tag) {
                  response.insert("tag", json::Integer(tag));
                }
                response.insert("error_code", json::Integer(0));
                this->send_message(response);
                return 0;
              }
              case NotFound:
                this->send_error(tag, 1, "The specified order was not found.");
                return 0;
              case InsufficientFunds:
                this->send_error(tag, 4, "You have insufficient funds.");
                return 0;
              case TooFast:
                this->send_error(tag, 6, "You are sending orders too rapidly.");
                return 0;
              case InvalidArgument:
                // can happen if the trade engine rounds the requested price down to zero
                this->send_error(tag, 8, "Price must not be zero.");
                return 0;
              case NotAllowed:
                this->send_error(tag, 9, "Post-only order with these parameters would result in an immediate match.");
                return 0;
              default:
                throw std::ios_base::failure("received illegal opcode");
            }
          });
        }
      }
    } else if (request_method == "CancelOrder") {
      if (order_bucket.take(1) == 0) {
        this->send_error(tag, 6, "You are sending orders too rapidly.");
      } else if (~user_id == 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else {
        struct CancelOrder cmd;
        auto request_order_id_ptr = request.find("id");
        auto request_tonce_ptr = request.find("tonce");
        if (request_order_id_ptr) {
          cmd.order_id = narrow_check<id_t>(*request_order_id_ptr->as_integer());
          cmd.opcode = CancelOrder;
        } else if (request_tonce_ptr) {
          cmd.tonce = narrow_check<uint64_t>(*request_tonce_ptr->as_integer());
          cmd.opcode = CancelOrderByTonce;
        }
        if (!request_order_id_ptr == !request_tonce_ptr) {
          this->send_error(tag, 8, "You must specify either order ID or tonce.");
        } else {
          cmd.user_id = user_id;
          this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
            std::thread::id this_id = std::this_thread::get_id();
            switch (*static_cast<const core::Opcode *>(buf)) {
              case Success: {
                if (n != 0) {
                  return sizeof(struct CancelOrderReply);
                }
                auto &reply = *static_cast<const struct CancelOrderReply *>(buf);
                if (elog.debug_enabled()) {
                  elog.debug() << ">> " << reply << std::endl;
                }
                {
                  std::lock_guard<std::mutex> transient_orders_lock(transient_orders_mutex);
                  transient_orders.erase(reply.order.order_id);
                }
                json::Object response = order_to_json(reply.order);
                if (tag) {
                  response.insert("tag", json::Integer(tag));
                }
                response.insert("error_code", json::Integer(0));
                this->send_message(response);
                return 0;
              }
              case NotFound:
                if (elog.trace_enabled()) {
                  elog.trace() << "Thread(" << this_id << ") CancelOrderReply: " << NotFound
                               << " code=" << 1 << " tag=" << tag << std::endl;
                } else if (elog.debug_enabled()) {
                  elog.debug() << "CancelOrderReply: " << NotFound
                               << " code=" << 1 << " tag=" << tag << std::endl;
                }
                this->send_error(tag, 1, "The specified order was not found.");
                return 0;
              case NotAllowed:
                this->send_error(tag, 1, "Modifying this order is not allowed.");
                return 0;
              default:
                throw std::ios_base::failure("received illegal opcode");
            }
          });
        }
      }
    } else if (request_method == "SetUserFeeTables") {
      if (user_id != 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else {
        struct SetUserFeeTables cmd;
        cmd.opcode = SetUserFeeTables;
        cmd.user_id = narrow_check<id_t>(*request.get("user_id").as_integer());
        cmd.maker_table_id = narrow_check<uint16_t>(*request.get("maker_table_id").as_integer());
        cmd.taker_table_id = narrow_check<uint16_t>(*request.get("taker_table_id").as_integer());
        this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
          switch (*static_cast<const core::Opcode *>(buf)) {
            case Success: {
              if (n != 0) {
                return sizeof(struct SetUserFeeTablesReply);
              }
              auto &reply = *static_cast<const struct SetUserFeeTablesReply *>(buf);
              if (elog.debug_enabled()) {
                elog.debug() << ">> " << reply << std::endl;
              }
              json::Object response;
              if (tag) {
                response.insert("tag", json::Integer(tag));
              }
              response.insert("error_code", json::Integer(0));
              this->send_message(response);
              return 0;
            }
            case NotFound:
              this->send_error(tag, 1, "The specified user or fee table was not found.");
              return 0;
            default:
              throw std::ios_base::failure("received illegal opcode");
          }
        });
      }
    } else if (request_method == "GetTradeVolume") {
      if (info_bucket.take(1) == 0) {
        this->send_error(tag, 6, "You are making information requests too rapidly.");
      } else if (~user_id == 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else {
        struct GetTradeVolume cmd;
        cmd.opcode = GetTradeVolume;
        cmd.user_id = user_id;
        cmd.asset_id = narrow_check<asset_t>(*request.get("asset").as_integer());
        this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
          switch (*static_cast<const core::Opcode *>(buf)) {
            case Success: {
              if (n != 0) {
                return sizeof(struct GetTradeVolumeReply);
              }
              auto &reply = *static_cast<const struct GetTradeVolumeReply *>(buf);
              if (elog.debug_enabled()) {
                elog.debug() << ">> " << reply << std::endl;
              }
              json::Object response;
              if (tag) {
                response.insert("tag", json::Integer(tag));
              }
              response.insert("error_code", json::Integer(0));
              response.insert("volume", json::Integer(reply.volume));
              this->send_message(response);
              return 0;
            }
            default:
              throw std::ios_base::failure("received illegal opcode");
          }
        });
      }
    } else if (request_method == "CancelAllOrders") {
      if (order_bucket.take(1) == 0) {
        this->send_error(tag, 6, "You are sending orders too rapidly.");
      } else if (~user_id == 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else {
        struct CancelAllOrders cmd;
        cmd.opcode = CancelAllOrders;
        cmd.user_id = user_id;
        this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
          switch (*static_cast<const core::Opcode *>(buf)) {
            case Success: {
              auto &reply = *static_cast<const struct CancelAllOrdersReply *>(buf);
              if (n != 0) {
                return sizeof(struct CancelAllOrdersReply) + (n < sizeof(struct CancelAllOrdersReply) ? 0 : reply.orders_count * sizeof(struct Order));
              }
              if (elog.debug_enabled()) {
                elog.debug() << ">> " << reply << std::endl;
              }
              {
                std::lock_guard<std::mutex> transient_orders_lock(transient_orders_mutex);
                transient_orders.clear();
              }
              json::Object response;
              if (tag) {
                response.insert("tag", json::Integer(tag));
              }
              response.insert("error_code", json::Integer(0));
              json::Array response_orders;
              response_orders->reserve(reply.orders_count);
              for (size_t i = 0; i < reply.orders_count; ++i) {
                response_orders.insert(order_to_json(reply.orders[i]));
              }
              response.insert("orders", std::move(response_orders));
              this->send_message(response);
              return 0;
            }
            default:
              throw std::ios_base::failure("received illegal opcode");
          }
        });
      }
    } else if (request_method == "WatchOrders") {
      asset_pair_t asset_pair;
      asset_pair.first = narrow_check<asset_t>(*request.get("base").as_integer());
      asset_pair.second = narrow_check<asset_t>(*request.get("counter").as_integer());
      auto book_ptr = get_book(asset_pair);
      if (book_ptr) {
        if (*request.get("watch").as_boolean()) {
          auto range = orders_watches.equal_range(asset_pair);
          if (range.first == range.second) {
            auto pair = book_ptr->add_orders_client(this);
            orders_watches.emplace_hint(range.first, asset_pair, pair.first);
            if (tag) {
              pair.second.insert("tag", json::Integer(tag));
            }
            pair.second.insert("error_code", json::Integer(0));
            this->send_message(pair.second);
          } else {
            this->send_error(tag, 2, "You are already watching the order book for the specified asset pair.");
          }
        } else {
          auto watch_itr = orders_watches.find(asset_pair);
          if (watch_itr == orders_watches.end()) {
            this->send_error(tag, 1, "You are not watching the order book for the specified asset pair.");
          } else {
            book_ptr->remove_orders_client(watch_itr->second);
            orders_watches.erase(watch_itr);
            json::Object response;
            if (tag) {
              response.insert("tag", json::Integer(tag));
            }
            response.insert("error_code", json::Integer(0));
            this->send_message(response);
          }
        }
      } else {
        this->send_error(tag, 1, "You specified an invalid asset pair.");
      }
    } else if (request_method == "WatchTicker") {
      asset_pair_t asset_pair;
      asset_pair.first = narrow_check<asset_t>(*request.get("base").as_integer());
      asset_pair.second = narrow_check<asset_t>(*request.get("counter").as_integer());
      auto book_ptr = get_book(asset_pair);
      if (book_ptr) {
        if (*request.get("watch").as_boolean()) {
          auto range = ticker_watches.equal_range(asset_pair);
          if (range.first == range.second) {
            auto pair = book_ptr->add_ticker_client(this);
            ticker_watches.emplace_hint(range.first, asset_pair, pair.first);
            if (tag) {
              pair.second.insert("tag", json::Integer(tag));
            }
            pair.second.insert("error_code", json::Integer(0));
            this->send_message(pair.second);
          } else {
            this->send_error(tag, 2, "You are already watching the ticker for the specified asset pair.");
          }
        } else {
          auto watch_itr = ticker_watches.find(asset_pair);
          if (watch_itr == ticker_watches.end()) {
            this->send_error(tag, 1, "You are not watching the ticker for the specified asset pair.");
          } else {
            book_ptr->remove_ticker_client(watch_itr->second);
            ticker_watches.erase(watch_itr);
            json::Object response;
            if (tag) {
              response.insert("tag", json::Integer(tag));
            }
            response.insert("error_code", json::Integer(0));
            this->send_message(response);
          }
        }
      } else {
        this->send_error(tag, 1, "You specified an invalid asset pair.");
      }
    } else if (request_method == "TransferBalance") {
      if (~user_id == 0) {
        this->send_error(tag, 7, "You are not authenticated.");
      } else if (uplink->protocol_version() < 1) {
        this->send_error(tag, 7, "Trade engine lacks support for this command.");
      } else {
        struct TransferBalance cmd;
        cmd.opcode = TransferBalance;
        cmd.from_user_id = narrow_check<id_t>(*request.get("from_user_id").as_integer());
        cmd.to_user_id = narrow_check<id_t>(*request.get("to_user_id").as_integer());
        cmd.asset_id = narrow_check<asset_t>(*request.get("asset").as_integer());
        cmd.amount = narrow_check<uint64_t>(*request.get("amount").as_integer());
        cmd.from_seq_num = narrow_check<uint64_t>(*request.get("from_seq").as_integer());
        cmd.to_seq_num = narrow_check<uint64_t>(*request.get("to_seq").as_integer());
        if (cmd.from_user_id == cmd.to_user_id) {
          this->send_error(tag, 8, "Must transfer between different users.");
        } else if (cmd.amount == 0) {
          this->send_error(tag, 8, "Amount must not be zero.");
        } else {
          this->do_privileged(tag, cmd.asset_id, [this, tag, cmd]() {
            this->do_request(tag, cmd, [this, tag](const void *buf, size_t n) -> size_t {
              switch (*static_cast<const core::Opcode *>(buf)) {
                case Success: {
                  if (n != 0) {
                    return sizeof(struct TransferBalanceReply);
                  }
                  auto &reply = *static_cast<const struct TransferBalanceReply *>(buf);
                  if (elog.debug_enabled()) {
                    elog.debug() << ">> " << reply << std::endl;
                  }
                  json::Object response;
                  if (tag) {
                    response.insert("tag", json::Integer(tag));
                  }
                  response.insert("error_code", json::Integer(0));
                  response.insert("from_balance", json::Integer(reply.from_balance));
                  response.insert("to_balance", json::Integer(reply.to_balance));
                  this->send_message(response);
                  return 0;
                }
                case NotFound:
                  this->send_error(tag, 1, "There is no such user.");
                  return 0;
                case OutOfSequence:
                  this->send_error(tag, 3, "Bad sequence number.");
                  return 0;
                case InsufficientFunds:
                  this->send_error(tag, 4, "There are insufficient funds in the specified account.");
                  return 0;
                default:
                  throw std::ios_base::failure("received illegal opcode");
              }
            });
          });
        }
      }
    } else {
      this->send_error(tag, 8, "unknown method");
    }
  }

  void Client::schedule_ping() noexcept {
    /*
    scheduler.call_at(next_ping_time, [weak_this = std::weak_ptr<Client>(shared_this)]() noexcept {
      if (auto shared_this = weak_this.lock()) {
        if (shared_this->next_ping_time <= std::chrono::steady_clock::now()) {
          auto this_ptr = shared_this.get();
          this_ptr->enqueue_work([shared_this = std::move(shared_this)]() noexcept {
            if (shared_this->next_ping_time <= std::chrono::steady_clock::now()) {
              std::lock_guard<std::mutex> send_lock(shared_this->send_mutex);
              if (shared_this->next_ping_time <= std::chrono::steady_clock::now()) {
                try {
                  //shared_this->send(Ping, nullptr, 0);
                }
                catch (...) {
                }
              }
            }
            shared_this->schedule_ping();
          });
        } else {
          shared_this->schedule_ping();
        }
      }
    });
     */
  }

  void Client::reschedule_ping() noexcept {
    next_ping_time = std::chrono::steady_clock::now() + ping_interval;
  }


  void Client::
  do_request(intmax_t tag, const void *msg, size_t n, std::function<size_t(const void *, size_t)> &&callback) {
    std::lock_guard<std::mutex> callback_queue_lock(callback_queue_mutex);
    //if (uplink->do_request(msg, n, shared_this)) {
    if (uplink->do_request(msg, n, shared_this)) {
      callback_queue.push(std::move(callback));
    } else {
      this->send_error(tag, 6, "The trading engine is too overloaded to process your request at this time.");
    }
  }

  void Client::do_privileged(intmax_t tag, asset_t asset_id, std::function<void(void)> &&action) {
    int authorized;
    {
      std::lock_guard<std::mutex> asset_authority_lock(asset_authority_mutex);
      if (asset_authority) {
        authorized = asset_authority->find(asset_id) == asset_authority->end() ? 0 : 1;
      } else {
        authorized = -1;
      }
    }
    if (authorized > 0) {
      action();
    } else if (authorized < 0) {
      struct GetAssetAuthority cmd;
      cmd.opcode = GetAssetAuthority;
      cmd.user_id = user_id;
      this->do_request(tag, cmd, [this, tag, asset_id, action = std::move(action)](const void *buf, size_t n) -> size_t {
        switch (*static_cast<const core::Opcode *>(buf)) {
          case Success: {
            auto &reply = *static_cast<const struct GetAssetAuthorityReply *>(buf);
            if (n != 0) {
              return sizeof(struct GetAssetAuthorityReply) + (n < sizeof(struct GetAssetAuthorityReply) ? 0 : reply.assets_count * sizeof(asset_t));
            }
            if (elog.debug_enabled()) {
              elog.debug() << ">> " << reply << std::endl;
            }
            bool authorized = false;
            {
              std::lock_guard<std::mutex> asset_authority_lock(asset_authority_mutex);
              asset_authority.reset(new std::set<asset_t>());
              for (size_t i = 0; i < reply.assets_count; ++i) {
                asset_authority->insert(reply.asset_ids[i]);
                if (asset_id == reply.asset_ids[i]) {
                  authorized = true;
                }
              }
            }
            if (authorized) {
              action();
            } else {
              this->send_error(tag, 7, "You are not an authority for the specified asset.");
            }
            return 0;
          }
          default:
            throw std::ios_base::failure("received illegal opcode");
        }
      });
    } else {
      this->send_error(tag, 7, "You are not an authority for the specified asset.");
    }
  }

  void Client::switch_user(id_t user_id) {
    if (user_id != this->user_id) {
      {
        std::lock_guard<std::mutex> transient_orders_lock(transient_orders_mutex);
        if (!transient_orders.empty()) {
          struct CancelOrder cmd;
          cmd.opcode = CancelOrderNoReply;
          cmd.user_id = this->user_id;
          for (auto order_id : transient_orders) {
            cmd.order_id = order_id;
            if (elog.debug_enabled()) {
              elog.debug() << "<< " << cmd << std::endl;
            }
            uplink->send_message(&cmd, sizeof cmd, true);
          }
          transient_orders.clear();
        }
      }
      std::lock_guard<std::shared_mutex> clients_wrlock(clients_rwlock);
      if (~this->user_id) {
        auto range = users_clients.equal_range(this->user_id);
        for (auto itr = range.first; itr != range.second; ++itr) {
          if (itr->second == this) {
            users_clients.erase(itr);
            break;
          }
        }
      }
      this->user_id = user_id;
      if (~user_id) {
        users_clients.insert({user_id, this});
      }
    }
  }



}