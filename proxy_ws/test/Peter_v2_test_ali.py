import asyncio
import websockets
import json
import requests
import time
import random
import datetime as dt

# test env
wss_url   = 'wss://api-test-v2.coinflex-cn.com/v2/websocket'
https_url = 'https://api-test-v2.coinflex-cn.com/v2/account/auth/trading/login'

# # dev env
# wss_url   = 'wss://api-dev-v2.coinflex-cn.com/v2/websocket'
# https_url = 'https://api-dev-v2.coinflex-cn.com/v2/account/auth/trading/login'

market = "BTC-USD-200626-LIN"
#market = "BTC-USD-SWAP-LIN"
#market = 'BTC-USD-SPR-QP-LIN'
#market = 'BTC-USD-REPO-LIN'


login = 'peter.chan+v2_test1@coinflex.com'
passwd= 'peter.test'
#login = "siang.xu+test1@coinflex.com"
#passwd ="coinflex"
#API    key: 3b207a63-b872-47f3-a85b-ba95fafc8b51
#API Secret: 73f1982f-cde5-44d6-b236-e65466377d3c

client_order_id = int(time.time()) * 1000 + 1
headers = {'content-type': 'application/json'}
data={"email":login, "password": passwd}

ws = None
logined = False

#res=requests.post("https://api-dev-v2.coinflex-cn.com/v2/account/auth/trading/login",headers=headers,data=json.dumps(data))
res=requests.post(https_url, headers=headers,data=json.dumps(data))
cf_token=res.json()["data"]["token"]

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
    global ws, logined
    while not logined:
        await asyncio.sleep(0.05)

    print(f"Start listening to notice messages...")
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
              await websocket.send(json.dumps({"op":"login","data":{"x-cf-token":str(cf_token)}}))
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

            rnd_bid_px = random.randint(8700, 8800);             rnd_ask_px = random.randint(8700, 8800)
            rnd_bid_qty = random.randint(1000, 9000);            rnd_ask_qty = random.randint(1000, 9000)

            await websocket.send(json.dumps(placeOrder( "BUY",  rnd_bid_qty, rnd_bid_px)))
            await websocket.send(json.dumps(placeOrder( "SELL", rnd_ask_qty, rnd_ask_px)))
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