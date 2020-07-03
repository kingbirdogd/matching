#include "common/cli.h"
#include "common/codec.h"
//#include "common/base64.h"
//#include "common/scheduler.h"
#include "Uplink.hpp"

//extern "C" {
//#include <libwebsockets.h>
//}
#include <string.h>
#include <signal.h>
#include <chrono>
#include <netinet/tcp.h>
#include <iostream>
#include <thread>
#include <shared_mutex>
#include "common/log.h"
#include "common/scheduler.h"
#include "core.h"
//#include "proxy_lws_utils.hpp"
//#include "Server.hpp"

#define LWS_PLUGIN_STATIC
//#include "proxy_lws_protocol.hpp"

//using njson = nlohmann::json;

Log elog(Log::INFO);

std::unordered_map<std::string, int> _client_connections_map;
std::mutex _client_connections_map_mutex;
int _max_concurrent_connection;

namespace proxy {

  using namespace core;
  using core::id_t;

  //  static bool fee_control_allowed = false;
//  static bool skip_auth_allowed = false;
//  static Scheduler<std::chrono::steady_clock> scheduler;
  std::shared_mutex books_rwlock;
  //std::map<asset_pair_t, Book> books;

  bool fee_control_allowed = false;
  bool skip_auth_allowed = false;
  Scheduler<std::chrono::steady_clock> scheduler;

  class Uplink;

//  static Uplink *uplink;
//  static Selector *selector;
  Uplink *uplink;
  Selector *selector;

  //static uint8_t cookie_secret[16];
  uint8_t cookie_secret[16];



}


//static struct lws_protocols protocols[] = {
//    //{NULL, NULL, "default", "1"},
//    //{ "http", callback_http_test, sizeof(struct per_session_data__minimal), 256, 0, NULL,0 },
//    LWS_PLUGIN_PROTOCOL_MINIMAL,
//    //{ "http", callback_minimal, 0, 0 },
//    { NULL, NULL, 0, 0 } /* terminator */
//};
//
//static const struct lws_protocol_vhost_options pvo_opt = {
//    NULL,
//    NULL,
//    "default",
//    "1"
//};
//
//static const struct lws_protocol_vhost_options pvo = {
//    NULL,
//    &pvo_opt,
//    "callback_minimal",
//    ""
//};
//
////static const lws_retry_bo_t retry = {
////    .secs_since_valid_ping = 3,
////    .secs_since_valid_hangup = 10,
////};
//
//static const lws_retry_bo_t retry = []{
//    lws_retry_bo_t tmp{};
//    tmp.secs_since_valid_ping = 3;
//    tmp.secs_since_valid_hangup = 10;
//    return tmp;
//}();
//
//static int interrupted;
//
//static const struct lws_http_mount mount = {
//    /* .mount_next */		NULL,		/* linked-list "next" */
//    /* .mountpoint */		"/",		/* mountpoint URL */
//    /* .origin */			"./mount-origin",  /* serve from dir */
//    /* .def */			"index.html",	/* default filename */
//    /* .protocol */			NULL,
//    /* .cgienv */			NULL,
//    /* .extra_mimetypes */		NULL,
//    /* .interpret */		NULL,
//    /* .cgi_timeout */		0,
//    /* .cache_max_age */		0,
//    /* .auth_mask */		0,
//    /* .cache_reusable */		0,
//    /* .cache_revalidate */		0,
//    /* .cache_intermediaries */	0,
//    /* .origin_protocol */		LWSMPRO_FILE,	/* files in a dir */
//    /* .mountpoint_len */		1,		/* char count */
//    /* .basic_auth_login_file */	NULL,
//};

//void sigint_handler(int sig)
//{
//  interrupted = 1;
//}



int main(int argc, const char **argv)
{
  // Uplink to matching engine
  cli::Option<in_port_t> matching_port_option("matching_port", 'X');
  cli::Option<in_port_t> wss_port_option("wss_port", 'G');

  //cli::Option<in_port_t> zmq_ob_snapshot_port_option("zmq_ob_snapshot_port", 'E');
  //cli::Option<in_port_t> zmq_ob_diff_port_option("zmq_ob_diff_port", 'F');
  cli::Option<std::string> pulsar_host_url_option("pulsar_host_url", 'R');
  cli::Option<std::string> md_pub_snapshot_url_option("md_pub_snapshot_url", 'E');
  cli::Option<std::string> md_pub_diff_url_option("md_pub_diff_url", 'F');

  cli::Option<uint32_t> broadcast_ms_option("broadcast_ms", 'B');
  cli::Option<std::string> md_schema_option("md_schema_file", 's');
  cli::Option<> verbose_option("verbose", 'v');
  cli::Option<> version_option("version");
  cli::Option<> fee_control("allowFeeControl");
  cli::Option<> num_queue_option("oneQueue");
  cli::Option<std::string> inst_config_option("instConfig", 'c');
  cli::Option<> skip_auth_option("skipAuth");
  cli::Option<> trace_option("trace");
  cli::Option<uint16_t> max_connections_option("maxConnections");
  cli::Option<std::string> ip_address_header_option("ipAddressHeader");

  argc = cli::parse(argc, (char **)argv, { &pulsar_host_url_option, &md_schema_option, &broadcast_ms_option, &md_pub_diff_url_option, &md_pub_snapshot_url_option, &matching_port_option, &wss_port_option, &verbose_option, &version_option, &fee_control, &inst_config_option, &skip_auth_option,
                                  &num_queue_option, &trace_option, &max_connections_option, &ip_address_header_option });
  if (version_option) {
    std::clog << "proxy" << ' ' << VERSION << std::endl;
    return 0;
  }
  if (argc != 2) {
    std::clog << "usage: " << (argc > 0 ? argv[0] : "proxy")
              << " [-v|--verbose] [--allowFeeControl] [-P <bind-port>] [--oneQueue] [-c <inst-config-json>]  <core-host>" << std::endl;
    return -1;
  }
  if (verbose_option) {
    elog.debug_stream_ptr = &std::clog;
  }
  if (trace_option) {
    elog.trace_stream_ptr = &std::clog;
  }
  if (fee_control) {
    proxy::fee_control_allowed = true;
  }
  uint32_t num_queue = num_queue_option ? 1 : std::thread::hardware_concurrency();
  if (num_queue == 1)
    std::clog << "Running with " << num_queue << " queue" << std::endl;
  else
    std::clog << "Running with more than 1 queue" << std::endl;
  if (inst_config_option) {
    //proxy::read_inst_config("/tmp/tmp.BejDc4c0An/contrib/nlohmann/instruments.json");
    std::string inst_config_fname = inst_config_option.value_or("");
    std::clog << "Reading instrument config from: " << inst_config_fname << std::endl;
    //proxy::read_inst_config(inst_config_fname);
    std::clog << "Finished reading instrument config from: " << inst_config_fname << std::endl;
  }
  if (skip_auth_option) {
    proxy::skip_auth_allowed = true;
    std::clog << "Skip authentication for testing" << std::endl;
  }
  /*
  {
    const char *secret_base64 = ::getenv("COOKIE_SECRET");
    if (!secret_base64) {
      std::clog << "missing " << "COOKIE_SECRET" << std::endl;
      return -1;
    }
    if (transcode<Base64Decoder>(proxy::cookie_secret, sizeof proxy::cookie_secret, secret_base64, std::strlen(secret_base64)) != sizeof proxy::cookie_secret) {
      std::clog << "invalid " << "COOKIE_SECRET" << std::endl;
      return -1;
    }
    ::unsetenv("COOKIE_SECRET");
  }
   */
  ::srand48(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  posix::signal(SIGPIPE, SIG_IGN);

  Selector selector_in, selector_out;
  proxy::selector = &selector_in;
  proxy::Uplink uplink(argv[1], selector_in, selector_out, num_queue,
      matching_port_option.value_or(core::CORE_PORT),
      pulsar_host_url_option.value(),
      md_pub_snapshot_url_option.value(),
      md_pub_diff_url_option.value(),
      //zmq_ob_snapshot_port_option.value_or(0),
      //zmq_ob_diff_port_option.value_or(0),
      broadcast_ms_option.value_or(1000),
      md_schema_option.value()
      );
  std::shared_ptr<proxy::Uplink> shared_uplink(proxy::uplink = &uplink);

  uint16_t default_n_max_connections = 4;
  const uint16_t max_connections = max_connections_option.value_or(default_n_max_connections);
  const std::string ip_address_identifier = ip_address_header_option.value_or("X-Forwarded-For");

  std::clog << "Maximum concurrent connections allowed per IP Address: " << max_connections << ". To change that value, set maxConnections command line argument" << std::endl;
  std::clog << "HTTP Header to identify IP Address: " << ip_address_identifier << ". To change that value, set ipAddressHeader command line argument" << std::endl;

  _max_concurrent_connection = max_connections;
//  proxy::Server server(ip_address_identifier, max_connections, port_option.value_or(proxy::Server::DEFAULT_PORT), num_queue);
//  uplink.connect([&selector_in, &server]() {
//    selector_in.add(server, &server, Selector::Flags::READABLE);
//  });
  uplink.connect([&selector_in]() {} );
  for (size_t n = std::max(num_queue, 1U); n > 0; --n) {
    std::thread(&Selectable::pump, std::ref(selector_in)).detach();
  }
  for (size_t n = std::max(num_queue, 1U); n > 0; --n) {
    if (num_queue == 1)
      std::thread(&WorkQueue::pump_1_queue).detach();
    else
      std::thread(&WorkQueue::pump).detach();
  }
  std::thread(&Selectable::pump, std::ref(selector_out)).detach();
  std::thread(&decltype(proxy::scheduler)::run, std::ref(proxy::scheduler)).detach();
  while (true) {
    sleep(1);
  }

//  // ==== libwebsocket setup ====
//  struct lws_context_creation_info info;
//  struct lws_context *context;
  const char *p;
  //| LLL_INFO | LLL_PARSER | LLL_HEADER | LLL_EXT | LLL_CLIENT | LLL_DEBUG
  //int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
  /* for LLL_ verbosity above NOTICE to be built into lws,
   * lws must have been configured and built with
   * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
  /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
  /* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
  /* | LLL_DEBUG */;

//  signal(SIGINT, sigint_handler);
//
//  if ((p = lws_cmdline_option(argc, argv, "-d")))
//    logs = atoi(p);
//
//  lws_set_log_level(logs, NULL);
//  lwsl_user("LWS minimal ws server | visit http://localhost:7681 (-s = use TLS / https)\n");
//
//  memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
//  info.port = wss_port_option.value_or(8080);
//  info.mounts = &mount;
//  info.protocols = protocols;
//  //info.pvo = &pvo;
//  info.vhost_name = "localhost";
//  info.ws_ping_pong_interval = 5;
//  info.timeout_secs = 1;
//  info.options =
//      LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
//
////  if (lws_cmdline_option(argc, argv, "-s")) {
////    lwsl_user("Server using TLS\n");
////    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
////    info.ssl_cert_filepath = "localhost-100y.cert";
////    info.ssl_private_key_filepath = "localhost-100y.key";
////  }
////
////  if (lws_cmdline_option(argc, argv, "-h"))
////    info.options |= LWS_SERVER_OPTION_VHOST_UPG_STRICT_HOST_CHECK;
////
////  if (lws_cmdline_option(argc, argv, "-v"))
////    info.retry_and_idle_policy = &retry;
//
//  context = lws_create_context(&info);
//  if (!context) {
//    lwsl_err("lws init failed\n");
//    return 1;
//  }
//
//  while (n >= 0 && !interrupted)
//    n = lws_service(context, 1000);
//
//  lws_context_destroy(context);

  return 0;
}


