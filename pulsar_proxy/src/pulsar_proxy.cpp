#include <matching_tcp_client.hpp>
#include <iostream>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <thread>
#include <msg_generated.h>
//#include <zmq.hpp>
#include <iomanip>
#include <unistd.h>
#include <csignal>
#include <fbs_helper.hpp>
#include <pulsar/Client.h>
//#include <lib/LogUtils.h>
#include "common/log.h"
#include <flatbuffers/util.h>
#include <flatbuffers/idl.h>

Log elog(Log::INFO);

//DECLARE_LOG_OBJECT()

using namespace CoinflexV2;
using namespace pulsar;

std::unordered_map<unsigned long long, unsigned long long> client_to_engine_id_map;
//std::mutex iomutex;
static bool bPause = false;

void signal_handler( int signal_num ) {
  if (signal_num == SIGUSR1) {
    bPause = !bPause;
    if (bPause)
      elog.info() << "Paused receiving orders... " << std::endl;
    else
      elog.info() << "Resumed receiving orders..." << std::endl;
  }
}

void handle_order(const matching::order& o)
{
	std::string type = "";
	std::string side = "";
	std::string status = "";
	std::string time_condition = "";
	std::string action = "";
	std::string matched_type = "";
	if (matching::order::order_type::LIMIT == o.type)
	{
		type = "LIMIT";
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
	else if (o.order_state == matching::order::order_status_type::CANCELED_BY_USER)
	{
		status = "CANCELED_BY_USER";
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
	else if (o.order_state == matching::order::order_status_type::REJECT_BUY_STOP_TRIGGER_LARGE_THAN_STOP_LIMIT)
	{
		status = "REJECT_BUY_STOP_TRIGGER_LESS_THAN_STOP_LIMIT";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMIT)
	{
		status = "REJECT_SELL_STOP_TRIGGER_LESS_THAN_STOP_LIMIT";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_UNKNOW_ORDER_ACTION)
	{
		status = "REJECT_UNKNOW_ORDER_ACTION";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_QUANTITY_ZERO)
	{
		status = "REJECT_QUANTITY_ZERO";
	}
	else if (o.order_state == matching::order::order_status_type::REJECT_LIMITE_ORDER_WITH_MARKET_PRICE)
	{
		status = "REJECT_LIMITE_ORDER_WITH_MARKET_PRICE";
	}
  else if (o.order_state == matching::order::order_status_type::CANCELED_ALL_BY_AUCTION)
  {
    status = "CANCELED_ALL_BY_AUCTION";
  }
  else if (o.order_state == matching::order::order_status_type::CANCELED_PARTIAL_BY_AUCTION)
  {
    status = "CANCELED_PARTIAL_BY_AUCTION";
  }
  else if (o.order_state == matching::order::order_status_type::REJECT_AUCTION_SUPPORT_BUY_SELL_ONLY)
  {
    status = "REJECT_AUCTION_SUPPORT_BUY_SELL_ONLY";
  }
  else
  {
    status = "CANCELED_BY_AMEND";
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
  else if (matching::order::order_time_condition::MAKER_ONLY_REPRICE == o.time_condition)
  {
    time_condition = "MAKER_ONLY_REPRICE";
  }
  else if (matching::order::order_time_condition::AUCTION == o.time_condition)
  {
    time_condition = "AUCTION";
  }
  else
	{
		time_condition = "EXPIRY";
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
	//client_to_engine_id_map[o.client_order_id] = o.order_id;
  {
    //std::lock_guard<std::mutex> lockGuard(iomutex);
    elog.info()
        << "account_id:" << o.account_id
        << ",market_id:" << o.market_id
        << ",action:" << action
        << ",side:" << side
        << ",time_condition:" << time_condition
        << ",order_id:" << o.order_id
        << ",client_order_id:" << o.client_order_id
        << ",quantity:" << o.quantity
        << ",display_quantity:" << o.display_quantity
        << ",remain_quantity:" << o.remain_quantity
        << ",price:" << o.price
        << ",buy_stop_trigger_price:" << o.buy_stop_trigger_price
        << ",buy_stop_limit_price:" << o.buy_stop_limit_price
        << ",sell_stop_trigger_price:" << o.sell_stop_trigger_price
        << ",sell_stop_limit_price:" << o.sell_stop_limit_price
        << ",last_match_price:" << o.last_match_price
        << ",last_match_quantity:" << o.last_match_quantity
        << ",last_matched_order_id:" << o.last_matched_order_id
        << ",last_matched_order_id2:" << o.last_matched_order_id2
        << ",matched_id:" << o.matched_id
        << ",status:" << status
        << ",matched_type:" << matched_type
        << ",timestamp_epoch_ms:" << o.timestamp_epoch_ms
        << std::endl;
  }
}

int main(int iArgc, char** pszArgv)
{
  signal(SIGUSR1, signal_handler);
  //flatbuffers::IDLOptions opts;
  //opts.strict_json = true;
  //opts.output_default_scalars_in_json = true;
  //flatbuffers::Parser parser(opts);

  if (7 != iArgc)
	{
		//std::cout << "usage: " << pszArgv[0] << " host <port[1,65535]> <zmq_port[1,65535]> <order_status_pub_port[1,65535]>" << std::endl;
    elog.info() << "usage: " << pszArgv[0] << " host <port[1,65535]> pulsar_url order_in_url order_out_url" << std::endl;
		return -1;
	}
	std::string host = pszArgv[1];
	int iPort = std::atoi(pszArgv[2]);
	if (iPort < 1 || iPort > 65535)
	{
    elog.info() << "usage: " << pszArgv[0] << " host <port[1,65535]> <zmq_port[1,65535]> <order_status_pub_port[1,65535]>" << std::endl;
		return -2;
	}

  std::string pulsar_host   = pszArgv[3];
  std::string order_in_url  = pszArgv[4];
  std::string order_out_url = pszArgv[5];
  std::string md_schema_file = pszArgv[6];
  //unsigned long long market_id = std::atoi(pszArgv[4]);

  Client client(pulsar_host.c_str());

  Producer producer;
//  ProducerConfiguration config = ProducerConfiguration();
//  config.setBatchingEnabled(true);
//  // TODO: Batch Builder?
//  config.setBatchingMaxAllowedSizeInBytes(128*1024*1024);
//  config.setBatchingMaxMessages(200000);
//  config.setPartitionsRoutingMode(ProducerConfiguration::PartitionsRoutingMode::RoundRobinDistribution);
//  // TODO: Switch Frequency?
//  config.setBatchingMaxPublishDelayMs(1);
//  //config.setCompressionType(CompressionLZ4);  // buggy in 2.6.0
//  config.setSendTimeout(6000);
//  config.setBlockIfQueueFull(true);

  Result result = ResultUnknownError;
  //std::string order_out_url = std::string("persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-") + std::to_string(market_id);
  while (result != ResultOk) {
    elog.info() << "Creating Producer: " << order_out_url << std::endl;
    //result = client.createProducer(order_out_url.c_str(), config, producer);
    result = client.createProducer(order_out_url.c_str(), producer);
    if (result != ResultOk) {
      elog.error() << "Error creating producer: " << result << std::endl;
      //return -1;
      sleep(1);
    }
  }

  result = ResultUnknownError;
  Consumer consumer;
  std::string market_id_str = order_in_url.substr(order_in_url.find_last_of('-') + 1);
  elog.info() <<"Market ID: " << market_id_str << std::endl;
  //std::string order_in_url = std::string("persistent://CF-V2/PRETRADE-ME/ORDER-IN-") + std::to_string(market_id);
  while (result != ResultOk) {
    elog.info() << "Creating Consumer: " << order_in_url << std::endl;
    result = client.subscribe(order_in_url.c_str(), std::string("pulsar_proxy-") + market_id_str, consumer);
    if (result != ResultOk) {
      elog.error() << "Failed to subscribe: " << result << std::endl;
      //return -1;
      sleep(1);
    }
  }

//  zmq::context_t ctx;
//  zmq::socket_t order_status_pub_sock(ctx, ZMQ_PUSH);
//  std::string order_status_pub_url = std::string("tcp://*:") + std::to_string(order_status_pub_port);
//  order_status_pub_sock.bind(order_status_pub_url);
//  std::cout << "order status publish bind to " << order_status_pub_url << std::endl;

	auto sPort = static_cast<unsigned short int>(iPort);
	matching_tcp_client c(host, sPort);
	c.set_connected([&]()
	{
    elog.info()  << "matching_tcp_client connected" << std::endl;
	});
	c.set_disconnected([&]()
	{
    elog.info()  << "matching_tcp_client disconnected" << std::endl;
	});


//  // Read schema
//  std::string schema_ok_file;
//  bool ok = flatbuffers::LoadFile(md_schema_file.c_str(), false, &schema_ok_file);
//  if (!ok) {
//    std::cout << "load file failed!" << std::endl;
//    return -1;
//  }
//  parser.Parse(schema_ok_file.c_str());

  c.set_on_order([&](const matching::order& o) {
    handle_order(o);
//    if (o.order_state == matching::order::CANCELED_BY_AMEND) {
//      elog.info() << "Not pushing CANCELED_BY_AMEND" << std::endl;
//      return;
//    }
    //auto fbs_buf = order_to_fbs_msg(o);  // (*buf, buf_sz)

//    {
//      //std::lock_guard<std::mutex> lockGuard(iomutex);
//      elog.info() << "size: " << fbs_buf.second  << std::endl;
//      if (fbs_buf.second > 0)
//        elog.info() << "Pulsar pushing:" << get_fbs_msg_order_as_string(fbs_buf.first.get()) << std::endl;
//    }
//    std::string jsongen = get_fbs_msg_order_as_json(fbs_buf.first.get()).dump();
//    std::string jsongen;
//    if (!GenerateText(parser, fbs_buf.first.get(), &jsongen)) {
//      elog.error() << "Couldn't serialize parsed data to JSON!" << std::endl;
//    }
//    jsongen.erase(std::remove(jsongen.begin(), jsongen.end(), '\n'), jsongen.end());
    std::string jsongen = order_to_json(o).dump();
    elog.info() << "Pulsar pushing:" << jsongen << std::endl;
    auto key = std::string("ACCOUNT-ID-")+std::to_string(o.account_id);
    Message msg = MessageBuilder().setOrderingKey(key).setPartitionKey(key).setContent(jsongen.c_str(), jsongen.size()).build();

    SendCallback cb;
    cb = [o, key, jsongen, &producer, cb](Result res, const MessageId& messageId) {
      elog.info() << "Message sent: " << res << " order_id:" << o.order_id << std::endl;
      if (res != ResultOk) {
        Message msg = MessageBuilder().setOrderingKey(key).setPartitionKey(key).setContent(jsongen.c_str(), jsongen.size()).build();
        elog.info() << "Retry sending: order_id:" << o.order_id << std::endl;
        producer.sendAsync(msg, cb);
        elog.info() << "Finished Retry sending: order_id:" << o.order_id << std::endl;
      }
    };
    elog.info() << "Ordering key: " << msg.getOrderingKey() << " Partition key: " << msg.getPartitionKey() << std::endl;
    producer.sendAsync(msg, cb);

    //order_status_pub_sock.send(zmq::const_buffer(fbs_buf.first, fbs_buf.second), zmq::send_flags::none);
  });

//	std::thread th([&]()
//	{
//		while (true)
//		{
//			c.run();
//		}
//	});

	/*
	matching::order o;
	o.side = matching::order::order_side::SELL;
	o.client_order_id = 1;
	o.quantity = 1000;
	o.display_quantity = 1000;
	o.price = 100;
	c.send(o);

	o.side = matching::order::order_side::BUY;
	o.client_order_id = 1;
	o.quantity = 980;
	o.display_quantity = 980;
	o.price = 101;
	c.send(o);
*/



  Message msg;
  ResultCallback cb_res = [](Result res){};
  ReceiveCallback cb_recv = [&consumer, &c, &cb_res](Result res, const Message& msg) {
    if (res == ResultOk) {
      elog.info() << "Received: " << msg << "  with payload length=" << msg.getLength() << std::endl;
      auto o = fbs_msg_to_order(msg.getData());
      consumer.acknowledgeAsync(msg, cb_res);
      elog.info() << get_order_as_string(o) << std::endl;
      c.send(o);
    }
  };

  while (true) {
    if (bPause) {
      elog.info() << "Sleeping for 2s..." << std::endl;
      sleep(2);
      continue;
    }
    //result = consumer.receive(msg, 1);
    consumer.receiveAsync(cb_recv);
    c.run();
  }
	//th.join();
  client.close();
	return 0;
}




