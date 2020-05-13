#pragma once

//#ifdef __cplusplus
//extern "C" {
//#endif
#include "libwebsockets.h"
//#include "string.h"
//#ifdef __cplusplus
//}
//#endif

//#if !defined (LWS_PLUGIN_STATIC)
//#define LWS_DLL
//#define LWS_INTERNAL
//#include <libwebsockets.h>
//#endif

#include "proxy_lws_struct.hpp"
#include "proxy_lws_utils.hpp"
#include "Client.hpp"
using namespace proxy;

namespace proxy { class Client; }

extern std::unordered_map<std::string, int> _client_connections_map;
extern std::mutex _client_connections_map_mutex;
extern int _max_concurrent_connection;
extern Log elog;

const char EXCEEDED_CONCURRENT_CONNECTIONS_LIMIT_CUSTOM_ERR_MSG[] = "Exceeded concurrent connections limit\n";

/* destroys the message when everyone has had a copy of it */
static void __minimal_destroy_message(void *_msg)
{
  struct msg *msg = (struct msg *)_msg;

  free(msg->payload);
  msg->payload = NULL;
  msg->len = 0;
}

static int callback_http_test(struct lws *wsi, enum lws_callback_reasons reason,
                            void *user, void *in, size_t len) {
  struct per_session_data__minimal *pss =
      (struct per_session_data__minimal *) user;
  struct per_vhost_data__minimal *vhd =
      (struct per_vhost_data__minimal *)
          lws_protocol_vh_priv_get(lws_get_vhost(wsi),
                                   lws_get_protocol(wsi));
  int m;

  switch (reason) {
    case LWS_CALLBACK_HTTP:
      break;
    case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE:
      break;
    case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
      break;
    default:
      break;
  }
  return 0;
}

static void get_headers_helper(struct lws *wsi, char *buf, size_t buf_sz) {
  int res = 0;
// Try to get the X-Forwarded-For header
//memset(&pss->x_forwarded_for, 0, sizeof(pss->x_forwarded_for));
  char forwarded_ip[INET6_ADDRSTRLEN + 1];
  res = lws_hdr_copy(wsi, forwarded_ip, sizeof(forwarded_ip), WSI_TOKEN_X_FORWARDED_FOR);
  if (res > 0) {
    lwsl_notice("Connection accepted from (X-Forwarded-For): %s\n", forwarded_ip);
    strncpy(buf, forwarded_ip, buf_sz);
    return;
  }

// Try to get the CF-Connecting-IP header
//memset(&pss->cf_connecting_ip, 0, sizeof(pss->x_forwarded_for));
  char connecting_ip[INET6_ADDRSTRLEN + 1];
  res = lws_hdr_copy(wsi, connecting_ip, sizeof(connecting_ip), WSI_TOKEN_CF_CONNECTING_IP);
  if (res > 0) {
    lwsl_notice("Connection accepted from (CF-Connecting-IP): %s\n", connecting_ip);
    strncpy(buf, connecting_ip, buf_sz);
    return;
  }
}

static int callback_minimal(struct lws *wsi, enum lws_callback_reasons reason,
                            void *user, void *in, size_t len) {
  struct per_session_data__minimal *pss =
      (struct per_session_data__minimal *)user;
  struct per_vhost_data__minimal *vhd =
      (struct per_vhost_data__minimal *)
          lws_protocol_vh_priv_get(lws_get_vhost(wsi),
                                   lws_get_protocol(wsi));
  int m;

  switch (reason) {
    case LWS_CALLBACK_PROTOCOL_INIT:
      vhd = (struct per_vhost_data__minimal *)lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
                                                                          lws_get_protocol(wsi),
                                                                          sizeof(struct per_vhost_data__minimal));
      vhd->context = lws_get_context(wsi);
      vhd->protocol = lws_get_protocol(wsi);
      vhd->vhost = lws_get_vhost(wsi);
      vhd->wsi_queue = new folly::UMPSCQueue<struct lws*, true, 10>();
    break;

    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
      //get_headers_helper(wsi);
      break; // 1st callback after client's connection
    case LWS_CALLBACK_WSI_CREATE: {
      //char hd[1024], *p = &hd[0];
      //lws_add_http_header_status(wsi, 200, (unsigned char **) &p, (unsigned char *) &hd[1023]);
      //lwsl_notice("hd %s\n", hd);

      //get_headers_helper(wsi);
    }
      break; // 2nd callback after client's connection
    case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED: {
      //char hd[1024], *p = &hd[0];
      //lws_add_http_header_status(wsi, 200, (unsigned char **) &p, (unsigned char *) &hd[1023]);
      //lwsl_notice("hd %s\n", hd);

      //get_headers_helper(wsi);
    }
      break; // 3rd callback after client's connection
    case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE: {                 // 4th callback after client's connection
      char tmp_buf[256];
      lws_hdr_copy(wsi, tmp_buf, sizeof(tmp_buf), WSI_TOKEN_HTTP);
      lwsl_notice("HTTP VERSION %s\n", tmp_buf);
      //lwsl_notice("wsi->http.request_version: %s\n", wsi->http.request_version);

      char hd[1024], *p = &hd[0];
      lws_add_http_header_status(wsi, 200, (unsigned char **) &p, (unsigned char *) &hd[1023]);
      lwsl_notice("hd %s\n", hd);

      char ip_char_array[16];
      get_headers_helper(wsi, ip_char_array, 16);
      elog.debug() << "ip char array=[" << ip_char_array << "]" << std::endl;
      std::string ip_str(ip_char_array);
      elog.debug() << "ip=[" << ip_str << "]" << std::endl;
      auto pos = ip_str.find(',');
      if (pos != std::string::npos) {  //no comma, return as is
        ip_str = ip_str.substr(0, pos);
      }
      //char *pos = std::find(ip_str, ip_str + 16, ',');      *pos = '\0';

      std::lock_guard<std::mutex> _guard(_client_connections_map_mutex);

      auto connections_map_itr = _client_connections_map.find(ip_str);
      if (connections_map_itr != std::end(_client_connections_map)) {
        //map contains already that ip address - check against the limit
        const int current_connections = connections_map_itr->second;
        if (current_connections < _max_concurrent_connection) {
          //We can still create extra connection
          connections_map_itr->second += 1;

          if (elog.debug_enabled()) {
            elog.debug() << "Updating concurrent connections count for IP address: " << ip_str << " to: " << connections_map_itr->second << std::endl;
          }
        } else {
          //No more connections allowed, reject
          if (elog.debug_enabled()) {
            elog.debug() << "No more concurrent connections allowed for IP address: " << ip_str << ". Connection denied" << std::endl;
          }
          lws_return_http_status(wsi, HTTP_STATUS_FORBIDDEN, EXCEEDED_CONCURRENT_CONNECTIONS_LIMIT_CUSTOM_ERR_MSG);
          //return {404, EXCEEDED_CONCURRENT_CONNECTIONS_LIMIT_CUSTOM_ERR_MSG};
          return 1;
        }
      } else {
        //map does not contain the ip address - add it
        if (elog.debug_enabled()) {
          elog.debug() << "Updating concurrent connections count for IP address: " << ip_str << " to: 1" << std::endl;
        }
        _client_connections_map.insert(std::make_pair(ip_str, 1));
      }

    }
//      if (lws_http_transaction_completed(wsi))
//        return -1;
      //bytearray(b'HTTP/1.0 400 Bad Request\r\ncontent-security-policy: default-src \'none\'; img-src \'self\' data: ; script-src \'self\'; font-src \'self\'; style-src \'self\'; connect-src \'self\' ws: wss:; frame-ancestors \'none\'; base-uri \'none\';form-action \'self\';\r\nx-content-type-options: nosniff\r\nx-xss-protection: 1; mode=block\r\nx-frame-options: deny\r\nreferrer-policy: no-referrer\r\ncontent-type: text/html\r\ncontent-length: 193\r\n\r\n<html><head><meta charset=utf-8 http-equiv="Content-Language" content="en"/><link rel="stylesheet" type="text/css" href="/error.css"/></head><body><h1>400</h1>Rejected connection\n</body></html>')
      break;
    case LWS_CALLBACK_HTTP_BIND_PROTOCOL: {
//      char hd[1024], *p = &hd[0];
//      lws_add_http_header_status(wsi, 200, (unsigned char **) &p, (unsigned char *) &hd[1023]);
//      lwsl_notice("hd %s\n", hd);

//      char msg[] = "Rejected Connection\n";
//      //lws_close_reason(wsi, LWS_CLOSE_STATUS_POLICY_VIOLATION, (unsigned char *) msg, sizeof msg);
//      lws_return_http_status(wsi, HTTP_STATUS_BAD_REQUEST, "Rejected connection");
////      if (lws_http_transaction_completed(wsi))
////        return -1;
//      return 1;
    }
      break; // 5th callback after client's connection
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: {
//      char hd[1024], *p = &hd[0];
//      lws_add_http_header_status(wsi, 200, (unsigned char **) &p, (unsigned char *) &hd[1023]);
//      lwsl_notice("hd %s\n", hd);


      //get_headers_helper(wsi);
//      char msg[] = "Rejected Connection\n";
//      lws_close_reason(wsi, LWS_CLOSE_STATUS_POLICY_VIOLATION, (unsigned char *) msg, sizeof msg);
//      lws_return_http_status(wsi, HTTP_STATUS_BAD_REQUEST, "Rejected connection");
//      if (lws_http_transaction_completed(wsi))
//        return -1;
//      return 1;
    }
      break;
      break; // 6th callback after client's connection
    case LWS_CALLBACK_ADD_HEADERS: {
//      char hd[1024], *p = &hd[0];
//      lws_add_http_header_status(wsi, 200, (unsigned char **) &p, (unsigned char *) &hd[1023]);
//      lwsl_notice("hd %s\n", hd);

      //get_headers_helper(wsi);
      //return -1;
    }
      break; // 7th callback after client's connection
    case LWS_CALLBACK_ESTABLISHED: {                         // 8th callback after client's connection
//      char hd[1024], *p = &hd[0];
//      lws_add_http_header_status(wsi, 200, (unsigned char **) &p, (unsigned char *) &hd[1023]);
//      lwsl_notice("hd %s\n", hd);

      //get_headers_helper(wsi);
      if (!vhd) {
        vhd = (struct per_vhost_data__minimal *) lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
                                                                             lws_get_protocol(wsi),
                                                                             sizeof(struct per_vhost_data__minimal));
        vhd->context = lws_get_context(wsi);
        vhd->protocol = lws_get_protocol(wsi);
        vhd->vhost = lws_get_vhost(wsi);
        //vhd->ctok  = ConsumerToken(vhd->wsi_queue);
      }
      lwsl_notice("using wsi %x\n", wsi);

      /* add ourselves to the list of live pss held in the vhd */
      lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
      pss->wsi = wsi;
      pss->last = vhd->current;

      // Get sockaddr
      struct sockaddr_storage addr;
      socklen_t len;
      get_sockaddr_storage(wsi, &addr, &len);

      // Get IP and port of client's connection
      char ipstr[INET6_ADDRSTRLEN + 1];
      int port;
      get_ip_and_port(&addr, ipstr, INET6_ADDRSTRLEN + 1, &port);

      // Save the sockaddr
      pss->addr = addr;
      int res = 0;

      // Try to get the X-Forwarded-For header
      memset(&pss->x_forwarded_for, 0, sizeof(pss->x_forwarded_for));
      char forwarded_ip[INET6_ADDRSTRLEN + 1];
      res = lws_hdr_copy(wsi, forwarded_ip, sizeof(forwarded_ip), WSI_TOKEN_X_FORWARDED_FOR);
      if (res > 0) {
        //lwsl_notice("Connection accepted from (X-Forwarded-For): %s\n", forwarded_ip);
        strncpy(pss->x_forwarded_for, forwarded_ip, sizeof(forwarded_ip));
      }

      // Try to get the CF-Connecting-IP header
      memset(&pss->cf_connecting_ip, 0, sizeof(pss->x_forwarded_for));
      char connecting_ip[INET6_ADDRSTRLEN + 1];
      res = lws_hdr_copy(wsi, connecting_ip, sizeof(connecting_ip), WSI_TOKEN_CF_CONNECTING_IP);
      if (res > 0) {
        //lwsl_notice("Connection accepted from (CF-Connecting-IP): %s\n", connecting_ip);
        strncpy(pss->cf_connecting_ip, connecting_ip, sizeof(connecting_ip));
      }

      lwsl_notice("Connection accepted from:[%s:%d], X-Forwarded-For:[%s], CF-Connecting-IP:[%s]\n",
                  ipstr, port, pss->x_forwarded_for, pss->cf_connecting_ip);

      Socket dummy_socket; sockaddr_in6 peer_addr; int v1 = 1; int num_queue = 1; // all dummy
      std::string ip_address = strlen(pss->x_forwarded_for) == 0 ? pss->cf_connecting_ip : pss->x_forwarded_for;
      pss->client_ptr = std::make_shared<Client>(std::move(dummy_socket), wsi, vhd, ip_address.c_str(), v1, num_queue, ip_address, _client_connections_map, _client_connections_map_mutex);
    }
      break;

    case LWS_CALLBACK_CLOSED:
      /* remove our closing pss from the list of live pss */
      if (pss)
        lws_ll_fwd_remove(struct per_session_data__minimal, pss_list,
                        pss, vhd->pss_list);
      if (pss->client_ptr) {
        lwsl_notice("setting to NULL for wsi %x\n", wsi);
        pss->client_ptr->set_wsi(NULL);
        pss->client_ptr.reset();
      }
      lws_cancel_service(lws_get_context(wsi));
      return -1;
      break;

    case LWS_CALLBACK_SERVER_WRITEABLE: {
      //if (!vhd->amsg.payload) break;
      //if (pss->last == vhd->current) break;
      if ((!pss) || (!pss->client_ptr)) {
        break;
      }
//      char close_msg[] = "Rejected Connection\n";
//      lws_close_reason(wsi, LWS_CLOSE_STATUS_POLICY_VIOLATION, (unsigned char *) "seeya", 5);
//      lws_cancel_service(lws_get_context(wsi));
//      lwsl_notice("Closing connection: %x\n", wsi);
//      return -1;


      json::Object msg;
      bool success = pss->client_ptr->reply_queue.try_dequeue(msg);
//      lwsl_notice("%s\n", msg);
      if (success) {
        std::stringstream ss;
        ss << msg;
        if (elog.debug_enabled()) {
          elog.debug() << "lws_write: " << ss.str() << std::endl;
        }

        const int N = 4096;
        char buf[LWS_PRE + N];
        memset(&buf[LWS_PRE], 0, N);
        int n = lws_snprintf(buf + LWS_PRE, N, "%s", ss.str().c_str());
        int m = lws_write(wsi, (unsigned char *) &buf[LWS_PRE], n, LWS_WRITE_TEXT);

//        /* notice we allowed for LWS_PRE in the payload already */
//        m = lws_write(wsi, ((unsigned char *) vhd->amsg.payload) +
//                           LWS_PRE, vhd->amsg.len, LWS_WRITE_TEXT);
//        if (m < (int) vhd->amsg.len) {
//          lwsl_err("ERROR %d writing to ws\n", m);
//          return -1;
//        }
        pss->last = vhd->current;
      }
      if (pss->client_ptr->reply_queue.size() > 0)
        lws_callback_on_writable(wsi);
      lws_cancel_service(lws_get_context(wsi));
    }
      break;

    case LWS_CALLBACK_RECEIVE: {
      if (vhd->amsg.payload)
        __minimal_destroy_message(&vhd->amsg);

      const size_t remaining = lws_remaining_packet_payload(wsi);
      if (remaining != len)
        lwsl_notice("Payload remaining: %d. Incoming buffer len: %d\n", remaining, len);

      if (len + pss->msg_sz > MSG_BUF_SZ) {
        lwsl_notice("Total message size too big (%d + %d) > %d. Discarding....\n", len, pss->msg_sz, MSG_BUF_SZ);
        pss->msg_sz = 0;
      } else {
        memcpy(&pss->msg_buf[pss->msg_sz], in, len);
        pss->msg_sz += len;

        if (!remaining && lws_is_final_fragment(wsi)) {
          //if (pss->msg_sz > 0) {   // we have some fragments
          //client->AppendMessageFragment(in, len, 0);
          //in = (void *)client->GetMessage();
          //len = client->GetMessageLength();
          //}
          pss->client_ptr->shared_this = pss->client_ptr;
          lwsl_notice("Client shared_ptr count=%d\n", pss->client_ptr.use_count());
          pss->client_ptr->received((const char *) pss->msg_buf, pss->msg_sz);
          pss->client_ptr->shared_this = nullptr;
          pss->msg_sz = 0;
          //client->ProcessMessage((char *)in, len, wsi);
          //client->ResetMessage()
        }
      }
//      else
//        client->AppendMessageFragment(in, len, remaining);

      //pss->client_ptr->received((const char*)in, len);
//		vhd->amsg.len = len;
//		/* notice we over-allocate by LWS_PRE */
//		vhd->amsg.payload = malloc(LWS_PRE + len);
//		if (!vhd->amsg.payload) {
//			lwsl_user("OOM: dropping\n");
//			break;
//		}
//
//		memcpy((char *)vhd->amsg.payload + LWS_PRE, in, len);
//		vhd->current++;
//
//		/*
//		 * let everybody know we want to write something on them
//		 * as soon as they are ready
//		 */
//		lws_start_foreach_llp(struct per_session_data__minimal **,
//				      ppss, vhd->pss_list) {
//			lws_callback_on_writable((*ppss)->wsi);
//		} lws_end_foreach_llp(ppss, pss_list);
      lws_cancel_service(lws_get_context(wsi));
    }
      break;
    case LWS_CALLBACK_EVENT_WAIT_CANCELLED: {
//      if (elog.debug_enabled()) {
//        elog.debug() << "LWS_CALLBACK_EVENT_WAIT_CANCELLED b4" << std::endl;
//      }
      //lwsl_notice("LWS_CALLBACK_EVENT_WAIT_CANCELLED\n");
      if (!vhd) break;

      struct lws *tmp_wsi = NULL;
//      if (elog.debug_enabled()) {
//        elog.debug() << "LWS_CALLBACK_EVENT_WAIT_CANCELLED" << std::endl;
//      }
      //bool success = vhd->wsi_queue.try_dequeue(vhd->ctok,tmp_wsi);
      //bool success = vhd->wsi_queue->try_dequeue(tmp_wsi);
      while (vhd->wsi_queue->try_dequeue(tmp_wsi)) {
        //if (success) {
        if (tmp_wsi) {
          lwsl_notice("setting writable on wsi %x\n", tmp_wsi);
          lws_callback_on_writable(tmp_wsi);
        }
      }
      //lws_cancel_service(lws_get_context(wsi));
    }
      break;
    default:
      break;
  }
//  if (wsi)
//    lws_cancel_service(lws_get_context(wsi));
  return 0;
}

#define LWS_PLUGIN_PROTOCOL_MINIMAL \
	{ \
		"callback_minimal", \
		callback_minimal, \
		sizeof(struct per_session_data__minimal), \
		256, \
		0, NULL, 0 \
	}

#if !defined (LWS_PLUGIN_STATIC)

/* boilerplate needed if we are built as a dynamic plugin */
/*
static const struct lws_protocols protocols[] = {
	LWS_PLUGIN_PROTOCOL_MINIMAL
};

int
init_protocol_minimal(struct lws_context *context,
		      struct lws_plugin_capability *c)
{
	if (c->api_magic != LWS_PLUGIN_API_MAGIC) {
		lwsl_err("Plugin API %d, library API %d", LWS_PLUGIN_API_MAGIC,
			 c->api_magic);
		return 1;
	}

	c->protocols = protocols;
	c->count_protocols = LWS_ARRAY_SIZE(protocols);
	c->extensions = NULL;
	c->count_extensions = 0;

	return 0;
}

int
destroy_protocol_minimal(struct lws_context *context)
{
	return 0;
}
 */
#endif
