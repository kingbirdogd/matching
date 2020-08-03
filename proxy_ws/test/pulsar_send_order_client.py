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


local_topic_prefix = 'persistent://prop/r1/ns1'
aliyun_dev_prefix  = 'persistent://CF-V2/ME-WS/ORDER-IN-'
posttrade_prefix   = 'persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-'

local_url       = 'localhost:6650'
aliyun_dev_url  = "172.21.11.79:6650"
aliyun_test_url = '172.21.21.221:6650'
aliyun_stg_url  = '172.42.13.79:6650'
aliyun_live_url = '172.41.11.101:6650'

# ==== Change the following as you need ====
topic_prefix = aliyun_dev_prefix
host_url     = aliyun_stg_url

#market_id = '2001021200925' # Futures
#market_id = '2001011000000' # Perp
#market_id = '2001051000000' # Spread
#market_id = '2001000000000' # Spot
market_id = '2001031000000' # #Repo
name = 'peter'
# ==========================================

producer_name = "producer-" + name
producer_id_name = "producer-id-" + name

client = pulsar.Client('pulsar://' + host_url )

producer = client.create_producer( #topic_prefix + '/ORDER-IN-' + market_id,
                    topic_prefix + market_id,
                    block_if_queue_full=True,
                    batching_enabled=True,
                    batching_max_publish_delay_ms=10,
                    properties={
                        "producer-name": producer_name,
                        "producer-id": producer_id_name
                    }
                )
random.seed()

# ==== Create order in flatbuffer format =====
builder = flatbuffers.Builder(1024)

co.OrderStart(builder)
co.OrderAddAccountId(builder, 4499257)
co.OrderAddMarketId(builder, int(market_id))
co.OrderAddPrice(builder, 102000000)
co.OrderAddQuantity(builder, 20000000)
co.OrderAddDisplayQuantity(builder, 20000000)

co.OrderAddOrderId(builder,160075308545858664)
#co.OrderAddOrderAction(builder, CoinflexV2.order_action_type.order_action_type.AMEND)
#co.OrderAddOrderAction(builder, CoinflexV2.order_action_type.order_action_type.NEW)
co.OrderAddOrderAction(builder, CoinflexV2.order_action_type.order_action_type.CANCEL)

co.OrderAddClientOrderId(builder, 1596020447111)
co.OrderAddSide(builder, CoinflexV2.order_side.order_side.BUY)
co.OrderAddType(builder, CoinflexV2.order_type.order_type.LIMITED)

order = co.OrderEnd(builder)

cm.MsgStart(builder)
cm.MsgAddPayloadType(builder, cp.Payload.Order)
cm.MsgAddPayload(builder, order)
msg = cm.MsgEnd(builder)
builder.Finish(msg)
buf = builder.Output()
# ============================================
# json_msg = r'''
# {
#   "payload_type": "Order",
#   "payload": {
#     "account_id": 541799,
#     "market_id": 2001011000000,
#     "price": 929050000000,
#     "quantity": 5000000000,
#     "display_quantity": 4900000000,
#     "remain_quantity": 4900000000,
#     "last_match_price": 929050000000,
#     "last_match_quantity": 100000000,
#     "order_id": 160063394736209069,
#     "client_order_id": 20000000005,
#     "last_matched_order_id": 160063394736209071,
#     "last_matched_order_id2": 160063394736209012,
#     "matched_id": 160063394736209072,
#     "buy_stop_trigger_price": 0,
#     "buy_stop_limited_price": 929050000000,
#     "sell_stop_trigger_price": 0,
#     "sell_stop_limited_price": 929050000000,
#     "side": "BUY",
#     "type": "LIMITED",
#     "time_condition": "MAKER_ONLY_REPRICE",
#     "order_action": "NEW",
#     "order_state": "PARTIAL_FILL",
#     "order_matched_type": "MAKER",
#     "timestamp_epoch_ms": 1595404588821,
#     "request_id": 464412
#   }
# }'''

cnt = 1
while True:
    workload = random.randint(1, 100)
    try:
        print(f'Sending Order...{cnt}'); cnt += 1
        #producer.send(json_msg.replace('\n', '').encode('utf-8'), None)
        producer.send(bytes(buf), None)
    except Exception as e:
        print("Failed to send message: %s", e)
    producer.flush()
    break
    time.sleep(1)

producer.close()