import asyncio
import websockets
import json
import requests
import time

# test env
wss_url   = 'wss://api-test-v2.coinflex-cn.com/v2/websocket'
https_url = 'https://api-test-v2.coinflex-cn.com/v2/account/auth/trading/login'

# dev env
wss_url   = 'wss://api-dev-v2.coinflex-cn.com/v2/websocket'
https_url = 'https://api-dev-v2.coinflex-cn.com/v2/account/auth/trading/login'

client_order_id = int(time.time()) * 1000 + 1
headers = {'content-type': 'application/json'}
data={"email":"siang.xu+test1@coinflex.com", "password": "coinflex"}

#res=requests.post("https://api-dev-v2.coinflex-cn.com/v2/account/auth/trading/login",headers=headers,data=json.dumps(data))
res=requests.post(https_url, headers=headers,data=json.dumps(data))

cf_token=res.json()["data"]["token"]
def placeOrder(side,quantity,price):
    global client_order_id
    send_order={"op":"placeorder", "data" : {"client_order_id":client_order_id, "market_code": "BTC-USD-200626-LIN", "side":str(side), "order_type":"LIMIT", "quantity":quantity, "time_in_force":"GTC", "limit_price":price}}
    print(f'{send_order}')
    client_order_id = client_order_id + 1
    return send_order
async def call_api():
    async with websockets.connect(wss_url) as websocket:
        await websocket.send(json.dumps({"op": "subscribe", "args": ["futures/depth:BTC-USD-200626-LIN"]}))
        while True:
            response = await websocket.recv()
            msg=json.loads(response)
            print(msg)
            if "nonce" in msg:
                await websocket.send(json.dumps({"op":"login","data":{"x-cf-token":str(cf_token)}}))
            if "event" in msg and msg["event"]=="login":
                print("________inputs______________________________________")
                print("______________________________________________")
                await websocket.send(json.dumps(placeOrder( "BUY", 1100, 60)))
                await websocket.send(json.dumps(placeOrder( "SELL", 1200, 210)))
                await websocket.send(json.dumps(placeOrder( "BUY", 1300, 50)))
                await websocket.send(json.dumps(placeOrder( "SELL", 1400, 210)))
                await websocket.send(json.dumps(placeOrder( "BUY", 1500, 60)))
                await websocket.send(json.dumps(placeOrder( "SELL", 1600, 210)))
                await websocket.send(json.dumps(placeOrder( "BUY", 1700, 60)))


def main():
    loop = asyncio.get_event_loop()
    try:
        loop.run_until_complete(call_api())
    except KeyboardInterrupt:
        pass
main()