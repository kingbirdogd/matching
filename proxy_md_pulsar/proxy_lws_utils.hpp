#ifndef ENGINE_PROXY_LWS_UTILS_HPP
#define ENGINE_PROXY_LWS_UTILS_HPP

#include "contrib/nlohmann/json.hpp"
#include <fstream>
#include <common/endian.h>
#include <arpa/inet.h>
#include "common/codec.h"
#include "common/base64.h"
#include "common/sha.h"
#include "Book.hpp"
#include "libwebsockets.h"

using njson = nlohmann::json;

namespace {
  struct AllUsers {
    bool _const operator()(core::id_t, uint8_t) const noexcept { return true; }
  };

  class ExcludeUser {
    const core::id_t exclude_user_id;
  public:
    explicit ExcludeUser(core::id_t exclude_user_id) noexcept : exclude_user_id(exclude_user_id) {}

  public:
    bool _pure operator()(core::id_t user_id, uint8_t) const noexcept {
      return user_id != exclude_user_id;
    }
  };

  class ExcludeUsers {
    const std::initializer_list<core::id_t> exclude_user_ids;
  public:
    explicit ExcludeUsers(std::initializer_list<core::id_t> exclude_user_ids) noexcept : exclude_user_ids(exclude_user_ids) {}

  public:
    bool _pure operator()(core::id_t user_id, uint8_t) const noexcept {
      return std::find(exclude_user_ids.begin(), exclude_user_ids.end(), user_id) == exclude_user_ids.end();
    }
  };

  template<typename V, typename Predicate = AllUsers>
  class CachingFormatter : private Predicate {
    const V &value;
    mutable BufferSink bs;
  public:
    template<typename... Args>
    explicit CachingFormatter(const V &value, Args &&...args) noexcept : Predicate(std::forward<Args>(args)...), value(value) {}

  public:
    std::string_view _pure operator()(core::id_t user_id, uint8_t api_version) const {
      if (!this->Predicate::operator()(user_id, api_version)) {
        return {};
      }
      if (!bs.grem()) {
        SinkBuf sb(bs);
        std::ostream os(&sb);
        os.exceptions(std::ios_base::badbit | std::ios_base::failbit);
        os << value << std::flush;
      }
      return {reinterpret_cast<const char *>(bs.gptr), bs.grem()};
    }

    void clear() noexcept {
      bs.clear();
    }
  };
}

namespace proxy {
  using core::asset_pair_t ;
  typedef std::array<uint64_t, 4> mpn224_t;
  extern uint8_t cookie_secret[16];

  static SHA1::digest_type _pure compute_cookie(id_t user_id) {
    class SHA1 sha1;
    sha1.write(cookie_secret, sizeof cookie_secret);
    be<uint64_t> user_id_be = user_id;
    sha1.write(&user_id_be, sizeof user_id_be);
    return sha1.digest();
  }

  static mpn224_t bytes_to_mpn224(const uint8_t (&in)[28]) {
    struct words_be_t {
      uint32_t msw;
      uint64_t lsw[3] __attribute__ ((packed));
    };
    static_assert(sizeof(words_be_t) == sizeof in, "");
    auto &words_be = reinterpret_cast<const words_be_t &>(in);
    return {{be64toh(words_be.lsw[2]), be64toh(words_be.lsw[1]), be64toh(words_be.lsw[0]), be32toh(words_be.msw)}};
  }

  static mpn224_t base64_to_mpn224(const char in[], size_t n_in) {
    uint8_t bytes[28];
    if (transcode<Base64Decoder>(bytes, sizeof bytes, in, n_in) != sizeof bytes) {
      throw std::invalid_argument("base64-encoded value has unexpected length");
    }
    return bytes_to_mpn224(bytes);
  }

  static mpn224_t base64_to_mpn224(const std::string &in) {
    return base64_to_mpn224(in.data(), in.size());
  }

  template<typename T>
  static void random_fill(T &t) {
    typedef int random_t;
    static_assert(sizeof(T) % sizeof(random_t) == 0, "");
    for (size_t i = 0; i < sizeof(T) / sizeof(random_t); ++i) {
      reinterpret_cast<random_t *>(&t)[i] = static_cast<random_t>(::mrand48());
    }
  }

  static inline bool _const check_muladddiv(uint64_t multiplicand, uint64_t multiplier, uint64_t addend, uint64_t divisor) noexcept {
    uint64_t rax, rdx;
    __asm__ (".ifnc %2,%%rax\n\tmulq %2\n\t.else\n\tmulq %3\n\t.endif" : "=a,a" (rax), "=d,d" (rdx) : "?0,?rm" (multiplicand), "?rm,?0" (multiplier) : "cc");
    __asm__ ("addq %2, %0\n\tadcq $0, %1" : "+a" (rax), "+d" (rdx) : "g" (addend) : "cc");
    return rdx < divisor;
  }

  void read_inst_config(std::string fname);
  int get_sockaddr_storage(struct lws *wsi, struct sockaddr_storage *addr, socklen_t *len);
  int get_ip_and_port(struct sockaddr_storage *addr, char* ipstr, int ip_len, int *port);

}

#endif //ENGINE_PROXY_LWS_UTILS_HPP
