import flatbuffers
import random
import time
import pulsar, _pulsar
import struct

#import CoinflexV2.*

import CoinflexV2.Admin
import CoinflexV2.Msg as cm
import CoinflexV2.Order as co
import CoinflexV2.Payload as cp
import CoinflexV2.Ver
import CoinflexV2.order_side
import CoinflexV2.order_action_type
import CoinflexV2.order_type


local_topic_prefix = 'persistent://prop/r1/ns1'
aliyun_dev_prefix  = 'persistent://CF-V2/ME-WS'
posttrade_prefix   = 'persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-'

local_url       = 'localhost:6650'
aliyun_dev_url  = "172.21.11.79:6650"
aliyun_test_url = '172.21.21.221:6650'
aliyun_stg_url  = '172.42.13.79:6650'
aliyun_live_url = '172.41.11.101:6650'

# ==== Change the following as you need ====
topic_prefix = posttrade_prefix
host_url     = aliyun_test_url

#market_id = '2001021200925' # Futures
market_id = '2001011000000' # Perp
#market_id = '2001051000000' # Spread
#market_id = '2001000000000' # Spot
#market_id = '2001031000000' # #Repo
name = 'peter'
# ==========================================

subscription_name = "subscription-" + name
consumer_name = 'consumer-' + name
consumer_id_name = 'consumer-id-' + name

client = pulsar.Client('pulsar://' + host_url )

consumer = client.subscribe(
                            topic_prefix + market_id,
                            subscription_name,
                            properties={
                                "consumer-name": consumer_name,
                                "consumer-id": consumer_id_name
                            })
# consumer = client.subscribe(
#                             "persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001011000000-message-processor-DLQ",
#                             "message-processor",
#                             consumer_type=_pulsar.ConsumerType.Shared,
#                             properties={
#                                 "consumer-name": consumer_name,
#                                 "consumer-id": consumer_id_name
#                             })


def print_order(o) :
  print(f"version={o.Version()},"
        f"account_id={o.AccountId()},"
        f"market_id={o.MarketId()},"
        f"price={o.Price()},"
        f"quantity={o.Quantity()},"
        f"display_quantity={o.DisplayQuantity()},"
        f"remain_quantity={o.RemainQuantity()},"
        f"last_match_price={o.LastMatchPrice()},"
        f"last_match_quantity={o.LastMatchQuantity()},"
        f"order_id={o.OrderId()},"
        f"client_order_id={o.ClientOrderId()},"
        f"last_matched_order_id={o.LastMatchedOrderId()},"
        f"last_matched_order_id2={o.LastMatchedOrderId2()},"
        f"matched_id={o.MatchedId()},"
        f"buy_stop_trigger_price={o.BuyStopTriggerPrice()},"
        f"buy_stop_limited_price={o.BuyStopLimitedPrice()},"
        f"sell_stop_trigger_price={o.SellStopTriggerPrice()},"
        f"sell_stop_limited_price={o.SellStopLimitedPrice()},"
        f"side={o.Side()},"
        f"type={o.Type()},"
        f"time_condition={o.TimeCondition()},"
        f"order_action={o.OrderAction()},"
        f"order_state={o.OrderState()},"
        f"order_matched_type={o.OrderMatchedType()},"
        f"timestamp_epoch_ms={o.TimestampEpochMs()}"
        )

if __name__ == '__main__':
    cnt = 0
    while True:
        buf = consumer.receive()
        #consumer.acknowledge(buf)

        # cnt += 1
        # if cnt == 1:
        #   continue

        print(f'recv {len(buf.data())} bytes from pulsar')
        print(f'publish_timestamp = {buf.publish_timestamp()} ')
        print(f'data = {buf.data()} ')
        #print(f'event_timestamp = {buf.event_timestamp()} ')
        #print(f'properties = {buf.properties()} ')
        #print(f'message_id = {buf.message_id()} ')
        #print(f'partition_key = {buf.partition_key()} ')
        #print(f'redelivery_count = {buf.redelivery_count()} ')
        #print(f'topic_name = {buf.topic_name()} ')
        #print(f'value = {buf.value()} ')
        #print(f'data = {buf.data()} ')
        #continue

        # msg = cm.Msg.GetRootAsMsg(buf.data(),0)
        # # ubyte = struct.unpack('B', buf.data()[0])[0]
        # # msg = cm.Msg.GetRootAsMsg(ubyte, 0)
        # payload_type = msg.PayloadType()
        # if payload_type == CoinflexV2.Payload.Payload.Order:
        #     order = co.Order()
        #     order.Init(msg.Payload().Bytes, msg.Payload().Pos)
        #     print_order(order)



