#include "proxy_lws_protocol.hpp"
#include "Client.hpp"

using namespace proxy;



//void __minimal_destroy_message(void *_msg)
//{
//  struct msg *msg = (struct msg *)_msg;
//
//  free(msg->payload);
//  msg->payload = NULL;
//  msg->len = 0;
//}
//
//int callback_minimal(struct lws *wsi, enum lws_callback_reasons reason,
//                            void *user, void *in, size_t len) {
//  struct per_session_data__minimal *pss =
//      (struct per_session_data__minimal *)user;
//  struct per_vhost_data__minimal *vhd =
//      (struct per_vhost_data__minimal *)
//          lws_protocol_vh_priv_get(lws_get_vhost(wsi),
//                                   lws_get_protocol(wsi));
//  int m;
//
//  switch (reason) {
//    case LWS_CALLBACK_PROTOCOL_INIT:
//      vhd = (struct per_vhost_data__minimal *)lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
//                                                                          lws_get_protocol(wsi),
//                                                                          sizeof(struct per_vhost_data__minimal));
//      vhd->context = lws_get_context(wsi);
//      vhd->protocol = lws_get_protocol(wsi);
//      vhd->vhost = lws_get_vhost(wsi);
//      break;
//
//    case LWS_CALLBACK_ESTABLISHED: {
//      /* add ourselves to the list of live pss held in the vhd */
//      lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
//      pss->wsi = wsi;
//      pss->last = vhd->current;
//
//      // Get sockaddr
//      struct sockaddr_storage addr;
//      socklen_t len;
//      get_sockaddr_storage(wsi, &addr, &len);
//
//      // Get IP and port of client's connection
//      char ipstr[INET6_ADDRSTRLEN + 1];
//      int port;
//      get_ip_and_port(&addr, ipstr, INET6_ADDRSTRLEN + 1, &port);
//
//      // Save the sockaddr
//      pss->addr = addr;
//      int res = 0;
//
//      // Try to get the X-Forwarded-For header
//      memset(&pss->x_forwarded_for, 0, sizeof(pss->x_forwarded_for));
//      char forwarded_ip[INET6_ADDRSTRLEN + 1];
//      res = lws_hdr_copy(wsi, forwarded_ip, sizeof(forwarded_ip), WSI_TOKEN_X_FORWARDED_FOR);
//      if (res > 0) {
//        //lwsl_notice("Connection accepted from (X-Forwarded-For): %s\n", forwarded_ip);
//        strncpy(pss->x_forwarded_for, forwarded_ip, sizeof(forwarded_ip));
//      }
//
//      // Try to get the CF-Connecting-IP header
//      memset(&pss->cf_connecting_ip, 0, sizeof(pss->x_forwarded_for));
//      char connecting_ip[INET6_ADDRSTRLEN + 1];
//      res = lws_hdr_copy(wsi, connecting_ip, sizeof(connecting_ip), WSI_TOKEN_CF_CONNECTING_IP);
//      if (res > 0) {
//        //lwsl_notice("Connection accepted from (CF-Connecting-IP): %s\n", connecting_ip);
//        strncpy(pss->cf_connecting_ip, connecting_ip, sizeof(connecting_ip));
//      }
//
//      lwsl_notice("Connection accepted from:[%s:%d], X-Forwarded-For:[%s], CF-Connecting-IP:[%s]\n",
//                  ipstr, port, pss->x_forwarded_for, pss->cf_connecting_ip);
//
//      Socket dummy_socket; sockaddr_in6 peer_addr; int v1 = 1; int num_queue = 1; // all dummy
//      std::string ip_address = strlen(pss->x_forwarded_for) == 0 ? pss->cf_connecting_ip : pss->x_forwarded_for;
//      pss->client_ptr = std::make_shared<Client>(std::move(dummy_socket), pss->wsi, vhd, ip_address.c_str(), v1, num_queue, ip_address, _client_connections_map, _client_connections_map_mutex);
//    }
//      break;
//
//    case LWS_CALLBACK_CLOSED:
//      /* remove our closing pss from the list of live pss */
//    lws_ll_fwd_remove(struct per_session_data__minimal, pss_list,
//                      pss, vhd->pss_list);
//      break;
//
//    case LWS_CALLBACK_SERVER_WRITEABLE:
//      if (!vhd->amsg.payload)
//        break;
//
//      if (pss->last == vhd->current)
//        break;
//
//      /* notice we allowed for LWS_PRE in the payload already */
//      m = lws_write(wsi, ((unsigned char *)vhd->amsg.payload) +
//                         LWS_PRE, vhd->amsg.len, LWS_WRITE_TEXT);
//      if (m < (int)vhd->amsg.len) {
//        lwsl_err("ERROR %d writing to ws\n", m);
//        return -1;
//      }
//
//      pss->last = vhd->current;
//      break;
//
//    case LWS_CALLBACK_RECEIVE:
//      if (vhd->amsg.payload)
//        __minimal_destroy_message(&vhd->amsg);
//
//      pss->client_ptr->received((const char*)in, len);
////		vhd->amsg.len = len;
////		/* notice we over-allocate by LWS_PRE */
////		vhd->amsg.payload = malloc(LWS_PRE + len);
////		if (!vhd->amsg.payload) {
////			lwsl_user("OOM: dropping\n");
////			break;
////		}
////
////		memcpy((char *)vhd->amsg.payload + LWS_PRE, in, len);
////		vhd->current++;
////
////		/*
////		 * let everybody know we want to write something on them
////		 * as soon as they are ready
////		 */
////		lws_start_foreach_llp(struct per_session_data__minimal **,
////				      ppss, vhd->pss_list) {
////			lws_callback_on_writable((*ppss)->wsi);
////		} lws_end_foreach_llp(ppss, pss_list);
//      break;
//
//    default:
//      break;
//  }
//
//  return 0;
//}
