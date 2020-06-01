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
sink = context.socket(zmq.PULL)
#sink.setsockopt(zmq.SUBSCRIBE, b"")
#sink.connect("tcp://localhost:22012")
sink.connect("tcp://18.162.39.80:22022")

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
        #workload = random.randint(1, 100)
        #sink.send_string(str(workload))
        buf = sink.recv()
        print(f'recv {len(buf)} bytes from PULL')

        msg = cm.Msg.GetRootAsMsg(buf,0)
        payload_type = msg.PayloadType()
        if payload_type == CoinflexV2.Payload.Payload.Order:
            order = co.Order()
            order.Init(msg.Payload().Bytes, msg.Payload().Pos)
            print_order(order)



