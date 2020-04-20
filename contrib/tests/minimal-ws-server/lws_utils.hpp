#ifndef ENGINE_LWS_UTILS_HPP
#define ENGINE_LWS_UTILS_HPP

#include <arpa/inet.h>
#include <libwebsockets.h>

int get_sockaddr_storage(struct lws *wsi, struct sockaddr_storage *addr, socklen_t *len) {
  // ==== print IP ====
  //socklen_t len;
  //struct sockaddr_storage addr;
  int fd;

  *len = sizeof(*addr);
  fd = lws_get_socket_fd(wsi);
  return getpeername(fd, (struct sockaddr *) addr, len);
}

int get_ip_and_port(struct sockaddr_storage *addr, char* ipstr, int ip_len, int *port) {
  //char ipstr[INET6_ADDRSTRLEN];
  //*ip_len = INET6_ADDRSTRLEN;
  int res = -1;

  // deal with both IPv4 and IPv6:
  if (addr->ss_family == AF_INET) {
    struct sockaddr_in *s = (struct sockaddr_in *) addr;
    *port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, ip_len);
    res = 0;
  } else { // AF_INET6
    struct sockaddr_in6 *s = (struct sockaddr_in6 *) &addr;
    *port = ntohs(s->sin6_port);
    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, ip_len);
    res = 0;
  }
  return res;
}


#endif //ENGINE_LWS_UTILS_HPP
