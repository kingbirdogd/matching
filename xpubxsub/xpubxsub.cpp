#include <iostream>
#include <string>
#include <sstream>
//#include "zhelpers.hpp"
#include <zmq.hpp>

int main (int argc, char *argv[])
{
  if (argc != 3) {
    std::cout << "usage: xpubxsub <xpub_port[1,65535]> <xsub_port[1,65535]> " << std::endl;
    return -1;
  }

  zmq::context_t context(1);

  //  Socket facing clients
  zmq::socket_t frontend (context, ZMQ_XSUB);
  frontend.setsockopt(ZMQ_RCVHWM, 1000);
  frontend.setsockopt(ZMQ_SNDHWM, 1000);

  std::stringstream frontend_url;
  frontend_url << "tcp://*:" << argv[2];
  std::cout << "xsub binding to " << frontend_url.str() << std::endl;
  frontend.bind(frontend_url.str().c_str());

  //  Socket facing services
  zmq::socket_t backend (context, ZMQ_XPUB);
  backend.setsockopt(ZMQ_RCVHWM, 1000);
  backend.setsockopt(ZMQ_SNDHWM, 1000);

  std::stringstream backend_url;
  backend_url << "tcp://*:" << argv[1];
  std::cout << "xpub binding to " << backend_url.str() << std::endl;
  backend.bind(backend_url.str().c_str());

  //  Start the proxy
  zmq::proxy(frontend,
             backend);

  return 0;
}