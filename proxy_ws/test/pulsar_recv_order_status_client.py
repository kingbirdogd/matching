import flatbuffers
import random
import time
import pulsar

#import CoinflexV2.*

import CoinflexV2.Admin
import CoinflexV2.Msg as cm
import CoinflexV2.Order as co
import CoinflexV2.Payload as cp
import CoinflexV2.Ver
import CoinflexV2.order_side
import CoinflexV2.order_action_type
import CoinflexV2.order_type


local_url = 'localhost:6650'
local_topic_prefix = 'persistent://prop/r1/ns1'

aliyuen_dev_url = "172.21.21.79:6650"
aliyuen_dev_prefix = 'persistent://CF-V2/ME-POSTTRADE'

aliyun_test_url = '172.21.21.221:6650'

# ==== Change the following as you need ====
topic_prefix = aliyuen_dev_prefix
host_url = aliyun_test_url

market_id = '2001031000000'
name = 'peter'
# ==========================================

subscription_name = "subscription-" + name
consumer_name = 'consumer-' + name
consumer_id_name = 'consumer-id-' + name

client = pulsar.Client('pulsar://' + host_url )

consumer = client.subscribe(topic_prefix + '/ORDER-OUT-' + market_id,
                            subscription_name,
                            properties={
                                "consumer-name": consumer_name,
                                "consumer-id": consumer_id_name
                            })

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
    while True:
        buf = consumer.receive()
        consumer.acknowledge(buf)

        print(f'recv {len(buf.data())} bytes from pulsar')

        msg = cm.Msg.GetRootAsMsg(buf.data(),0)
        payload_type = msg.PayloadType()
        if payload_type == CoinflexV2.Payload.Payload.Order:
            order = co.Order()
            order.Init(msg.Payload().Bytes, msg.Payload().Pos)
            print_order(order)



