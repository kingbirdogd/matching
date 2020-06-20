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
aliyuen_dev_prefix = 'persistent://CF-V2/PRETRADE-ME'

aliyun_test_url = '172.21.21.221:6650'

# ==== Change the following as you need ====
host_url = aliyun_test_url
topic_prefix = aliyuen_dev_prefix

market_id = '2001031000000'
name = 'peter'
# ==========================================

producer_name = "producer-" + name
producer_id_name = "producer-id-" + name

client = pulsar.Client('pulsar://' + host_url )

producer = client.create_producer(topic_prefix + '/ORDER-IN-' + market_id,
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
co.OrderAddAccountId(builder, 123456)
co.OrderAddMarketId(builder, 333)
co.OrderAddPrice(builder, 102)
co.OrderAddQuantity(builder, 2000)
co.OrderAddDisplayQuantity(builder, 2000)

co.OrderAddOrderId(builder,160041304405855872)
co.OrderAddOrderAction(builder, CoinflexV2.order_action_type.order_action_type.AMEND)
#co.OrderAddOrderAction(builder, CoinflexV2.order_action_type.order_action_type.NEW)

co.OrderAddClientOrderId(builder, 1322)
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

cnt = 1
while True:
    workload = random.randint(1, 100)
    try:
        print(f'Sending Order...{cnt}'); cnt += 1
        producer.send(bytes(buf), None)
    except Exception as e:
        print("Failed to send message: %s", e)
    producer.flush()
    break
    time.sleep(1)

producer.close()