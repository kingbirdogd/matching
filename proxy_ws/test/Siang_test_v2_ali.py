import asyncio
import websockets
import json
import requests
headers = {'content-type': 'application/json'}
data={"email":"siang.xu+test1@coinflex.com", "password": "coinflex"}
res=requests.post("https://api-dev-v2.coinflex-cn.com/v2/account/auth/trading/login",headers=headers,data=json.dumps(data))
cf_token=res.json()["data"]["token"]
def placeOrder(client,side,quantity,price):
    send_order={"op":"placeorder", "data" : {"client_order_id":client, "market_code": "BTC-USD-200626-LIN", "side":str(side), "order_type":"LIMIT", "quantity":quantity, "time_in_force":"GTC", "limit_price":price}}
    return send_order
async def call_api():
    async with websockets.connect('wss://api-dev-v2.coinflex-cn.com/v2/websocket') as websocket:
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
                await websocket.send(json.dumps(placeOrder(1, "BUY", 100, 50)))
                await websocket.send(json.dumps(placeOrder(2, "SELL", 100, 200)))
def main():
    loop = asyncio.get_event_loop()
    try:
        loop.run_until_complete(call_api())
    except KeyboardInterrupt:
        pass
main()