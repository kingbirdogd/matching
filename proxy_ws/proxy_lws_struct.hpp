#ifndef ENGINE_PROXY_LWS_STRUCT_HPP
#define ENGINE_PROXY_LWS_STRUCT_HPP

#include <memory>
//#include "contrib/concurrentqueue/concurrentqueue.h"
#include "folly/concurrency/UnboundedQueue.h"

#define MSG_BUF_SZ 256
//using namespace moodycamel;
//using namespace folly;

namespace proxy { class Client; }

/* one of these created for each message */

struct msg {
  void *payload; /* is malloc'd */
  size_t len;
};

/* one of these is created for each client connecting to us */

struct per_session_data__minimal {
  struct per_session_data__minimal *pss_list;
  struct lws *wsi;
  int last; /* the last message number we sent */

  struct sockaddr_storage addr;
  char x_forwarded_for[ INET6_ADDRSTRLEN + 1];
  char cf_connecting_ip[INET6_ADDRSTRLEN + 1];
  std::shared_ptr<proxy::Client> client_ptr;
  char msg_buf[MSG_BUF_SZ];
  size_t msg_sz;
};

/* one of these is created for each vhost our protocol is used with */

struct per_vhost_data__minimal {
  struct lws_context *context;
  struct lws_vhost *vhost;
  const struct lws_protocols *protocol;

  struct per_session_data__minimal *pss_list; /* linked-list of live pss*/

  struct msg amsg; /* the one pending message... */
  int current; /* the current message number we are caching */
  folly::UMPSCQueue<struct lws*, true, 10>  *wsi_queue = NULL;
  //ConcurrentQueue<struct lws*>  wsi_queue;
  //ConsumerToken ctok;
};


#endif //ENGINE_PROXY_LWS_STRUCT_HPP
