import flatbuffers
import zmq
import random
import time

#import CoinflexV2.*

import CoinflexV2.Admin
import CoinflexV2.Msg as cm
import CoinflexV2.Order as co
import CoinflexV2.Payload as cp
import CoinflexV2.Ver
import CoinflexV2.order_side
import CoinflexV2.order_action_type
import CoinflexV2.order_type

context = zmq.Context()

# Socket with direct access to the sink: used to syncronize start of batch
sink = context.socket(zmq.PUSH)
sink.connect("tcp://localhost:22001")

# Initialize random number generator
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

while True:
    workload = random.randint(1, 100)
    #sink.send_string(str(workload))
    sink.send(buf)
    time.sleep(1)