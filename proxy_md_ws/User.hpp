#ifndef ENGINE_USER_HPP
#define ENGINE_USER_HPP

#include "core.h"
#include <shared_mutex>

namespace proxy {

  struct User {
    core::PublicKey public_key;
  };

  static std::shared_mutex users_rwlock;
  static std::deque <User> users;

  static bool get_user_public_key(id_t user_id, core::PublicKey &public_key) {
    std::shared_lock <std::shared_mutex> users_rdlock(users_rwlock);
    if (user_id >= users.size()) {
      return false;
    }
    public_key = users[user_id].public_key;
    return true;
  }
}
#endif //ENGINE_USER_HPP
