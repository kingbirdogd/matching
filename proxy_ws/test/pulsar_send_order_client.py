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

topic_prefix = local_topic_prefix
host_url = local_url

client = pulsar.Client('pulsar://' + host_url )

producer = client.create_producer(topic_prefix + '/ORDER-IN-2001011000000',
                    block_if_queue_full=True,
                    batching_enabled=True,
                    batching_max_publish_delay_ms=10,
                    properties={
                        "producer-name": "test-producer-name",
                        "producer-id": "test-producer-id"
                    }
                )
random.seed()
builder = flatbuffers.Builder(1024)

co.OrderStart(builder)
co.OrderAddAccountId(builder, 123456)
co.OrderAddMarketId(builder, 333)
co.OrderAddPrice(builder, 101)
co.OrderAddQuantity(builder, 2000)
co.OrderAddDisplayQuantity(builder, 2000)
co.OrderAddClientOrderId(builder, 1)
co.OrderAddSide(builder, CoinflexV2.order_side.order_side.BUY)
co.OrderAddOrderAction(builder, CoinflexV2.order_action_type.order_action_type.NEW)
co.OrderAddType(builder, CoinflexV2.order_type.order_type.LIMITED)

order = co.OrderEnd(builder)

cm.MsgStart(builder)
cm.MsgAddPayloadType(builder, cp.Payload.Order)
cm.MsgAddPayload(builder, order)
msg = cm.MsgEnd(builder)
builder.Finish(msg)
buf = builder.Output()

cnt = 1
while True:
    workload = random.randint(1, 100)
    try:
        print(f'Sending Order...{cnt}'); cnt += 1
        producer.send(bytes(buf), None)
    except Exception as e:
        print("Failed to send message: %s", e)
    producer.flush()
    #sink.send_string(str(workload))
    #sink.send(buf)
    time.sleep(1)

producer.close()