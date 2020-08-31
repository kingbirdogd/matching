import asyncio
import websockets
import json
import requests
import time
import random
import datetime as dt
import base64
import hmac
import hashlib

# # # test env
# wss_url   = 'wss://api-test-v2.coinflex-cn.com/v2/websocket'
# https_url = 'https://api-test-v2.coinflex-cn.com/v2/account/auth/trading/login'
# api_key    = '3b207a63-b872-47f3-a85b-ba95fafc8b51'
# api_secret = '73f1982f-cde5-44d6-b236-e65466377d3c'

# # dev env
# wss_url   = 'wss://api-dev-v2.coinflex-cn.com/v2/websocket'
# https_url = 'https://api-dev-v2.coinflex-cn.com/v2/account/auth/trading/login'
# api_key    = 'ZeCafws/E911MGSa16+jCObJIIO33ZOx9Kv/ZeovTsk='
# api_secret = 'LS7wkx3K4pnJGIecuX44Y+R0iXZjmZ/E5Nqmjgjiutw='

# stage env
wss_url   = 'wss://v2stgapi.coinflex.com/v2/websocket'
https_url = 'https://v2stgapi.coinflex.com/v2/account/auth/trading/login'
api_key    = 'OFHrN+Kyi7M6ScAZGTYREBJQhKRme4ARUJN5QcQ+J4U='
api_secret = 'wxUTiLcIW068IDTYbsUFLOq33SEONuby063Yj4XmDxc='

# # lemon env
# wss_url   = 'wss://api-lemon-v2.coinflex-cn.com/v2/websocket'
# #https_url = 'https://api-dev-v2.coinflex-cn.com/v2/account/auth/trading/login'
# api_key    = '1yp3dBb2uajNqhrPMY/0mZqXpOxPm75oQsSOeeqo4hs='
# api_secret = 'JK9Skbdtd+tMaQpyXQwtPzljQ8T0BTQvPRv5bK1LI2U='

# # v2
# wss_url   = 'wss://v2api.coinflex.com/v2/websocket'
# https_url = 'https://v2api.coinflex.com/v2/account/auth/trading/login'
#'https://v2api.coinflex.com/v2/markets/public/markets/'

#market = "BTC-USD-200925-LIN"
#market = "BTC-USD-SWAP-LIN"
market = "BTC-USD"
#market = 'BTC-USD-SPR-QP-LIN'
#market = 'BTC-USD-REPO-LIN'
#market = 'FLEX-USD'#
#market = "USDT-USD-SWAP-LIN"

#market = "ETH-USD-200925-LIN"
#market = "ETH-USD-SWAP-LIN"
#market = 'ETH-USD-SPR-QP-LIN'
#market = 'ETH-USD-REPO-LIN'

bounds = { 'BTC' : [11940, 11999], 'ETH': [318, 328], 'USD': [98,102] }

client_order_id = int(time.time()) * 1000 + 1
headers = {'content-type': 'application/json'}
#data={"email":login, "password": passwd}

ws = None
logined = False

# res=requests.post(https_url, headers=headers,data=json.dumps(data))
# cf_token=res.json()["data"]["token"]

ts = str(int(time.time() * 1000))
sig_payload = (ts+'GET/auth/self/verify').encode('utf-8')
signature = base64.b64encode(hmac.new(api_secret.encode('utf-8'), sig_payload, hashlib.sha256).digest()).decode('utf-8')

msg_auth = \
{
  "op": "login",
  "tag": 1,
  "data": {
           "apiKey": api_key,
           "timestamp": ts,
           "signature": signature
          }
}


def placeOrder(side,quantity,price):
    global client_order_id
    send_order={"op":"placeorder", "data" : {"clientOrderId":client_order_id, "marketCode": market, "side":str(side), "orderType":"LIMIT", "quantity":quantity, "timeInForce":"GTC", "price":price}}
    print(f'{dt.datetime.now()} {send_order}')
    client_order_id = client_order_id + 1
    return send_order

def amendOrder(side,quantity,price, order_id):
    global client_order_id
    send_order = {"op": "modifyorder","data": {"orderId": order_id, "marketCode": market, "side": str(side),"orderType": "LIMIT", "quantity": quantity, "timeInForce": "GTC", "price": price}}
    print(f'{dt.datetime.now()} {send_order}')
    client_order_id = client_order_id + 1
    return send_order

async def get_reply_notice(sleep_s, bPrint=True):
    global ws, logined, market
    while not logined:
        await asyncio.sleep(0.05)

    print(f"Start listening to notice messages...")
    if ws and logined:
        #sub_msg = f'{{"op":"subscribe", "args":["order:{market}"],"tag":1}}'
        sub_msg = f'{{"op":"subscribe", "args":["futures/depth:{market}"],"tag":1}}'
        await ws.send(sub_msg)

    while True:
        if ws and logined:
            response = await ws.recv()
            msg = json.loads(response)
            if bPrint:
                print(f'{dt.datetime.now()} {msg}')

async def call_api():
    global ws, logined

    ws = await websockets.connect(wss_url)
    websocket = ws
    #async with ws as websocket:
    #await websocket.send(json.dumps({"op": "subscribe", "args": ["futures/depth:" + market]}))
    while True:
        if not logined:
            response = await websocket.recv()
            msg=json.loads(response)
            print(msg)
            if "nonce" in msg:
              await websocket.send(json.dumps(msg_auth))
              #await websocket.send(json.dumps({"op": "login", "data": {"x-cf-token": str(cf_token)}}))
              #await websocket.send(json.dumps({"op": "subscribe", "args": ["futures/depth:" + market]}))
              logined =True
              stopped =False
        if ("event" in msg and msg["event"]=="login") or logined :
            print("________inputs______________________________________")
            print("______________________________________________")
            #if not stopped:
              #await websocket.send(json.dumps(placeOrder( "BUY",  1200, -0.000800000))) ; stopped = True
              #await websocket.send(json.dumps(amendOrder("BUY", 1100, -0.000800000, 160041304405855875)));    stopped = True
            # await websocket.send(json.dumps(placeOrder( "SELL",  1100,  0.0001)))
            # await websocket.send(json.dumps(placeOrder( "BUY",  1300, -0.000500000)))
            # await websocket.send(json.dumps(placeOrder( "SELL", 1400,  0.002200000)))
            # await websocket.send(json.dumps(placeOrder( "BUY",  1500, -0.0006000000)))
            # await websocket.send(json.dumps(placeOrder( "SELL", 1600,  0.0021300000)))
            # await websocket.send(json.dumps(placeOrder( "BUY",  1700, -0.0007)))

            lbound = bounds[market[0:3]][0]
            ubound = bounds[market[0:3]][1]
            rnd_bid_px  = random.randint(lbound, ubound);        rnd_ask_px  = random.randint(lbound, ubound)
            if market[0:3] == 'USD' :
              rnd_bid_px /= 100
              rnd_ask_px /= 100
            rnd_bid_qty = random.randint(1, 3);                  rnd_ask_qty = random.randint(1, 3)

            #await websocket.send(json.dumps(placeOrder( "BUY",  rnd_bid_qty, rnd_bid_px)))
            #await websocket.send(json.dumps(placeOrder( "SELL", rnd_ask_qty, rnd_ask_px)))
        #break
        await asyncio.sleep(2)

def main():
    loop = asyncio.get_event_loop()
    try:
        loop.create_task(get_reply_notice(sleep_s=0.001, bPrint=True))
        loop.create_task(call_api())
        loop.run_forever()
        #loop.run_until_complete(call_api())
    except KeyboardInterrupt:
        pass
main()