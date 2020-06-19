import flatbuffers
import random
import time
import datetime as dt

import MdsMsg as mm
import pulsar

local_url = 'localhost:6650'
local_topic_prefix = 'persistent://prop/r1/ns1'

aliyuen_dev_url = "172.21.21.79:6650"
aliyuen_dev_prefix = 'persistent://CF-V2/ME-WS'

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
consumer = client.subscribe(topic_prefix + '/MD-SNAPSHOT-' + market_id,
                            subscription_name,
                            properties={
                                "consumer-name": consumer_name,
                                "consumer-id": consumer_id_name
                            })

def getStr_PxLevel(pxl, ind=2):
  ss = ind * " "
  outStr = f"[{pxl.Px()} {pxl.Qty()} {pxl.NumLiqOrders()} {pxl.NumOrders()}],"
  return outStr

def getStr_OrderBook(ob, ind=2):
  ss = ind*" "
  outStr = f"{ss}instrument_id={ob.InstrumentId()},\n" \
           f"{ss}timestamp={ob.Timestamp()},\n" \
           f"{ss}checksum={ob.Checksum()},\n" \
           f"{ss}seq_num={ob.SeqNum()},\n"
  outStr += f"{ss}bids={{\n"
  outStr += f"{(ind+2)*' '}"
  bids_len = ob.BidsLength()
  for i in range(0, bids_len):
    pxl = ob.Bids(i)
    outStr += getStr_PxLevel(pxl, ind+2)
  outStr += f"{ss}}},\n"

  outStr += f"{ss}asks={{\n"
  outStr += f"{(ind+2)*' '}"
  asks_len = ob.AsksLength()
  for i in range(0, asks_len):
    pxl = ob.Asks(i)
    outStr += getStr_PxLevel(pxl, ind+2)
  outStr += f"{ss}}},\n"

  return outStr

def getStr_MdsMsg(o, ind=2) :
  ss = ind*" "
  outStr = f"{ss}action={o.Action()},\n" \
           f"{ss}data={{\n"
  data_len = o.DataLength()
  for i in range(0,data_len):
    outStr += getStr_OrderBook(o.Data(i),ind+2)
         #f"OrderBook={o.MarketId()},"
  outStr += f"{ss}}}"
  return outStr

if __name__ == '__main__':
    while True:
        buf = consumer.receive()
        consumer.acknowledge(buf)
        print(f'{dt.datetime.now()} ',end='')
        print(f'recv {len(buf.data())} bytes from pulsar')

        msg = mm.MdsMsg.GetRootAsMdsMsg(buf.data(),0)
        print (getStr_MdsMsg(msg))



