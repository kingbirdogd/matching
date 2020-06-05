import asyncio
import websockets
import base64
import random
import requests
import json
import time
from ecdsa import ellipticcurve
from ecdsa import curves
from ecdsa import SigningKey
from hashlib import sha224
from hashlib import sha1
import base64
#import utils as ut
import logging
import sys
from _collections import deque

# import logging
# logger = logging.getLogger('websockets')
# logger.setLevel(logging.DEBUG)
# logger.addHandler(logging.StreamHandler())
#
# logging.getLogger('websockets.server').setLevel(logging.DEBUG)
# logging.getLogger('websockets.server').addHandler(logging.StreamHandler())
# logging.getLogger('websockets.client').setLevel(logging.DEBUG)
# logging.getLogger('websockets.client').addHandler(logging.StreamHandler())

HOST  = 'localhost'
PORT  = 8080
#HOST1 = 'ironmanapi1.coinflex.com'
#HOST2 = 'ironmanapi2.coinflex.com'
#HOST3 = 'ironmanapi3.coinflex.com'
#HOST  = '18.162.39.80'
#HOST  = 'chiaapi.coinflex.com'
#HOST  = 'lycheews.coinflex.com/test'
#HOST  = ''
#PORT  = 0
PORT1 = 8081
PORT2 = 8082
PORT3 = 8083
PORT4 = 8084
PORT5 = 8085
PROTOCOL = 'lws-minimal'

M = 100000000
BASE_ID    = 0xFFFE # 65534
COUNTER_ID = 0xFFFF # 65535

nProxies = 1
nTimes   = 100
#USER_IDS = [1, 2]
USER_IDS = range(1,2)
nUsers   = len(USER_IDS)

def or_bytes(abytes, bbytes):
    return bytes([a | b for a, b in zip(abytes[::-1], bbytes[::-1])][::-1])

class Args:
    def __init__(self, user_id, port):
        if port == 0:
            self.url = f"wss://{HOST}"  # websocket URL for LIVE
            #self.url = f"wss://{HOST}"  # websocket URL for LIVE
        else:
            self.port = ':' + str(port)
            #self.url  = f"ws://{HOST}{self.port}/v1"  # websocket URL for LIVE
            self.url  = f"ws://{HOST}{self.port}"  # websocket URL for LIVE
        self.id   = user_id  # this is core ID for your CoinFLEX account
        self.tag  = 1000 + user_id  # user_id = 1  ==> tag = 1001
        self.cookie ='YptI5VM7gq5XCEolVumsqDG412s='
        self.phrase = ''  # this is password for your CoinFLEX account

# Connects to the CoinFLEX /assets/ REST endpoint to retrieve the asset ID's
def get_assets():
    Assets = {}
    response = requests.get(url="https://webapi.coinflex.com/assets/",
                            headers={'Content-type': 'application/x-www-form-urlencoded'})
    asset_list = response.json()
    for item in asset_list:
        Assets[item['name']] = {}
        Assets[item['name']]['id'] = item['id']
        Assets[item['name']]['scale'] = item['scale']
    return Assets

#Assets = get_assets()

def get_markets():
    Markets = {}
    response = requests.get(url="https://webapi.coinflex.com/markets/",
                            headers={'Content-type': 'application/x-www-form-urlencoded'})
    market_list = response.json()
    for item in market_list:
        Markets[item['base']] = {}
        Markets[item['base']]['counter'] = item['counter']
        if item.get('start') != None:
            Markets[item['base']]['start'] = item['start']
        if item.get('expires') != None:
            Markets[item['base']]['expires'] = item['expires']
    return Markets

#Markets = get_markets()

def secp224k1():
    _a = 0x0000000000000000000000000000000000000000000000000000000000
    _b = 0x0000000000000000000000000000000000000000000000000000000005
    _p = 0x00fffffffffffffffffffffffffffffffffffffffffffffffeffffe56d
    _Gx = 0x00a1455b334df099df30fc28a169a467e9e47075a90f7e650eb6b7a45c
    _Gy = 0x007e089fed7fba344282cafbd6f7e319f7c0b0bd59e2ca4bdb556d61a5
    _r = 0x010000000000000000000000000001dce8d2ec6184caf0a971769fb1f7
    curve_secp224k1 = ellipticcurve.CurveFp(_p, _a, _b)
    generator_secp224k1 = ellipticcurve.Point(curve_secp224k1, _Gx, _Gy, _r)
    secp224k1_instance = curves.Curve(
        "SECP224k1",
        curve_secp224k1,
        generator_secp224k1,
        (1, 3, 132, 0, 20),
        "secp256k1"
    )
    return secp224k1_instance

def compute_ecdsa_signature(user_id, passphrase, server_nonce, client_nonce):
    # sys_random = random.SystemRandom()
    # ecdsa_nonce = sys_random.getrandbits(224)
    user_bytes = int(user_id).to_bytes(8, "big")
    message = b"".join([user_bytes, server_nonce, client_nonce])
    key = b"".join([user_bytes, passphrase])
    key_hash = sha224(key).digest()
    exponent = int.from_bytes(key_hash, "big", signed=False)
    # secp224k1 = secp224k1()
    priv_key = SigningKey.from_secret_exponent(exponent, curve=secp224k1(), hashfunc=sha224)
    r, s = priv_key.sign_deterministic(message, hashfunc=sha224,
                                       sigencode=lambda r, s, order: (r, s)
                                       )
    r = r.to_bytes(28, "big")
    s = s.to_bytes(28, "big")
    r = base64.b64encode(r).decode()
    s = base64.b64encode(s).decode()

    return r, s

def authenticate(tag, user_id, cookie, passphrase, server_nonce):
    client_nonce = random.getrandbits(16 * 8).to_bytes(16, "big")
    signature = compute_ecdsa_signature(user_id, passphrase.encode(), server_nonce, client_nonce)
    send_signature = {"tag": tag, "method": "Authenticate", "user_id": user_id, "cookie": cookie,
                      "nonce": base64.b64encode(client_nonce).decode(), "signature": signature}
    return send_signature

async def get_reply_notice(user_id, sleep_s, bPrint):
    global ws_map, bAuth_map, all_replies_map, ws1, ws2, ws3, bAuth1, bAuth2, bAuth3, all_replies1, all_replies2, all_replies3
    all_replies_map[user_id] = all_replies = {}
    bAuth = bAuth_map.get(user_id, False)
    while not bAuth:
        await asyncio.sleep(0.05)
        bAuth = bAuth_map.get(user_id, False)

    print(f"Start listening to {user_id}'s notice messages")
    ws = ws_map[user_id]
    while bAuth:
        if ws and ws.open and bAuth:
            listen_map[user_id] = True
            response = await ws.recv()
            msg = json.loads(response)
            if 'notice' in msg:
                if bPrint:
                    print(f'Notice({user_id}): {msg}')
            else:
                if bPrint:
                    print(f'Reply({user_id}): {msg}')
                # all_replies.append(msg)
        # print(f'user_id={user_id}, bAuth={bAuth}, ws={ws}')
        if bPrint:
            sys.stdout.flush()
        #await asyncio.sleep(sleep_s)

async def test(host, port, payload):
    global ws_map, listen_map, tonce, HOST, PORT
    sleep_s = 0.5
    HOST = host
    PORT = port

    # Login and authentication...
    for uid in USER_IDS:
        await connect_and_login(uid)
        await asyncio.sleep(sleep_s)
    print(f'{nUsers} users login successfully.')

    # Listening to reply/notice messages...
    while not all([x for x in listen_map.values()]):
        await asyncio.sleep(0.05)
    print(f'Listening to {nUsers} users reply/notice messages...')

    try:
        user_id_limit = USER_IDS[0]; ws_limit = ws_map[user_id_limit]
        #user_id_po = USER_IDS[1];  ws_po = ws_map[user_id_po]

        # ========================= Clear ==========================
        #payload_place_cxl  = {'order_action':"CANCEL",             'method':'PlaceOrder',  'order_id': 1589463760000000708, 'client_order_id': tonce}; tonce += 1; await send_order(user_id_limit, payload_place_cxl )
        #payload_place_buy  = {'order_action':'NEW', 'side':'BUY' , 'method':'PlaceOrder', "price":4000, 'quantity': 200000, 'client_order_id': tonce}; tonce += 1; await send_order(user_id_limit, payload_place_buy )
        #payload_place_sell = {'order_action':'NEW', 'side':'SELL', 'method':'PlaceOrder', "price":   1, 'quantity': 200000, 'client_order_id': tonce}; tonce += 1; await send_order(user_id_limit, payload_place_sell)
        #payload_place_buy  = {'order_action':'NEW', 'side':'BUY' , 'method':'PlaceOrder', "price":   2, 'quantity': 200000, 'client_order_id': tonce}; tonce += 1; await send_order(user_id_limit, payload_place_buy )
        # END ===================== Clear ==========================
        await send_order(user_id_limit, payload)

        # payload_place_sell = {'order_action':'NEW', 'side':'SELL'     , 'method':'PlaceOrder', "price":100, 'quantity': 20000, 'client_order_id': tonce}; tonce += 1; await send_order(user_id_limit, payload_place_sell)
        # payload_place_sell = {'order_action':'NEW', 'side':'SELL'     , 'method':'PlaceOrder', "price":100, 'quantity': 2000, 'client_order_id': tonce}; tonce += 1; await send_order(user_id_limit, payload_place_sell)
        # payload_place_buy  = {'order_action':'NEW', 'side':'BUY'      , 'method':'PlaceOrder', "price": 95, 'quantity': 2000, 'client_order_id': tonce}; tonce += 1; await send_order(user_id_limit, payload_place_buy )
        # payload_place_buy  = {'order_action':'NEW', 'side':'BUY'      , 'method':'PlaceOrder', "price": 99, 'quantity': 12000, 'client_order_id': tonce}; tonce += 1; await send_order(user_id_limit, payload_place_buy )
        # #payload_place_buy  = {'order_action':'NEW', 'side':'BUY'      , 'method':'PlaceOrder', "price":2000098, 'quantity': 20000, 'client_order_id': tonce}; tonce += 1; await send_order(user_id_limit, payload_place_buy )
        # payload_place_sell = {'order_action':'NEW', 'side':'SELL_STOP', 'method':'PlaceOrder',              'quantity':  2300, 'client_order_id': tonce,
        #                       'sell_stop_trigger_price': 98,
        #                       'sell_stop_limited_price': 97                                            }; tonce += 1; await send_order(user_id_limit, payload_place_sell)
        # payload_place_sell = {'order_action':'NEW', 'side':'SELL_STOP', 'method':'PlaceOrder',              'quantity':  2100, 'client_order_id': tonce,
        #                       'sell_stop_trigger_price': 80,
        #                       'sell_stop_limited_price': 78                                            }; tonce += 1; await send_order(user_id_limit, payload_place_sell)
        # payload_place_sell = {'order_action':'NEW', 'side':'SELL'     , 'method':'PlaceOrder', "price": 98, 'quantity': 2500, 'client_order_id': tonce}; tonce += 1; await send_order(user_id_limit, payload_place_sell)

        # payload_place_buy = {'method': 'PlaceOrder', "order_action": "NEW", "price": 101, 'quantity': 100, "client_order_id": tonce, 'side': 'BUY'}; tonce += 1
        # await send_order(user_id_limit, payload_place_buy)

        await asyncio.sleep(0.5)

        # payload_ticker_mod = {"method": "ModifyOrder", 'tonce': tonce-2, "quantity_delta" : 2}
        # await send_order(user_id_limit, payload_ticker_mod)

        # payload_ticker_cxl_stop = {"method": "CancelOrder", 'tonce': tonce-2}
        # ws = ws_map[user_id_limit]
        # await ws.send(json.dumps(payload_ticker_cxl_stop));

        # # Place Post-Only (Accept)
        # payload_place_post_only = {"method": "PlaceOrder", "base": BASE_ID, "counter": COUNTER_ID, "tag": 999, 'tonce': tonce,
        #                            "price": 99, 'quantity': -1100, "post_only": True  }; tonce += 1
        # await send_order(user_id_po, payload_place_post_only)
        #
        # # Place Post-Only (Reject)
        # payload_place_post_only = {"method": "PlaceOrder", "base": BASE_ID, "counter": COUNTER_ID, "tag": 999, 'tonce': tonce,
        #                            "price": 98, 'quantity': -1000, "post_only": True  }; tonce += 1
        # #await send_order(user_id_po, payload_place_post_only)
        pass

    except Exception as error:
        err_msg = 'Error: ' + str(time.time()) + ' ' + repr(error)
        print(err_msg)

async def auth(user_id):
    global ws_map, bAuth_map, args_map, tonce
    ws, bAuth = ws_map[user_id], False
    response = await ws.recv()
    msg = json.loads(response)
    print(msg)

    server_nonce = base64.b64decode(msg['nonce'])
    args = args_map[user_id]
    payload_auth = authenticate(args.tag, args.id, args.cookie, args.phrase, server_nonce)
    await ws.send(json.dumps(payload_auth))
    tonce += 1

    response = await ws.recv()
    msg = json.loads(response)
    print(msg)

    if 'error_code' in msg and msg['error_code'] == 0:
        print('Authentication Successful')
        bAuth = True
    else:
        print('Authentication Unsuccessful. Exiting...')
        bAuth = False
    bAuth_map[user_id] = bAuth

async def connect_and_login(user_id):
    global ws_map, args_map, listen_map, tonce
    listen_map[user_id] = False
    user_args = Args(user_id, PORT)
    args_map[user_id] = user_args
    print(f'Connecting to {user_args.url} ...')
    ws = await websockets.connect(user_args.url,
                                  #subprotocols=[PROTOCOL, ],
                                  extra_headers={'X-Forwarded-For':'123.45.45.56'})
    while not ws.open:
        ws = await websockets.connect(user_args.url,
                                      #subprotocols=[PROTOCOL, ],
                                      extra_headers={'X-Forwarded-For':'123.45.45.56'})
    ws_map[user_id] = ws
    #bAuth_map[user_id] = True
    try:
        await auth(user_id)
        while True:
            await asyncio.sleep(0.1)
            if bAuth_map[user_id]:
                print(f'User {user_id} login successfully.')
                break
        # await setup_orderbook(1, buy_px=99, buy_qty=1600, sell_px=100, sell_qty=3000)
    except Exception as error:
        err_msg = 'Error: ' + str(time.time()) + ' ' + repr(error)
        print(err_msg)

async def send_random_order(user_id, payload):
    global args_map, order_map, ws_map, tonce
    args = args_map[user_id]
    if   payload['method'] == 'PlaceOrder':
        payload['price']    = random.randint(80,120)
        payload['quantity'] = random.randint(-10,10) * 100
        payload['tag']      = args.tag
        payload['tonce']    = tonce
        order_map[user_id].append(payload)
        tonce += 1
    elif payload['method'] == 'CancelOrder':
        orders = order_map[user_id]
        if len(orders) == 0:
            print(f"User {user_id}'s orders is empty. Not cancelling any order...")
            return
        last_order = orders.pop()
        payload['tonce'] = last_order['tonce']

    await send_order(user_id, payload)

async def send_order(user_id, payload):
    global ws_map
    print(f'User {user_id} sending: {payload}')
    ws = ws_map[user_id]
    await ws.send(json.dumps(payload))

async def send_random_orders(nTimes, nUsers):
    # Need to change price/quantity/tag/tonce
    pl_order  = {"method": "PlaceOrder", "base": BASE_ID, "counter": COUNTER_ID,
                 "price": 105, 'quantity': 1000, "tag": 8888, 'tonce': 1}
    # Need to change tonce
    pl_cancel = {"method": "CancelOrder", 'tonce': 1}

    list_pl = [pl_order, pl_cancel]
    for i in range(nTimes+1):
        for uid in USER_IDS:
            idx = random.randint(0,len(list_pl)-1)
            pl  = list_pl[idx]
            await send_random_order(user_id=uid, payload=pl)
            #print(f'Will send {pl["method"]}')  #await asyncio.sleep(0.001)

async def test_stress(nUsers, sleep_s):
    global ws_map, listen_map, tonce

    # Login and authentication...
    for uid in USER_IDS:
        await connect_and_login(uid)
        await asyncio.sleep(sleep_s)
    print(f'{nUsers} users login successfully.')

    # Listening to reply/notice messages...
    while not all([x for x in listen_map.values()]):
        asyncio.sleep(0.05)
    print(f'Listening to {nUsers} users reply/notice messages...')

    # Send random orders
    await send_random_orders(nTimes=nTimes, nUsers=nUsers)

async def test_bkt_not_enough_balance_when_triggered():
    global ws1, ws2, ws3, tonce
    async with websockets.connect(args1.url) as ws1, \
            websockets.connect(args2.url) as ws2, \
            websockets.connect(args3.url) as ws3:
        if not ws1.open:            ws1 = await websockets.connect(args1.url); tonce += 1
        if not ws2.open:            ws2 = await websockets.connect(args2.url); tonce += 1
        if not ws3.open:            ws3 = await websockets.connect(args3.url); tonce += 1

        try:
            await auth1(ws1)
            await auth2(ws2)
            await auth3(ws3)
            while True:
                await asyncio.sleep(0.1)
                if bAuth1 and bAuth2 and bAuth3: break
            await setup_orderbook(1, buy_px=99, buy_qty=1600, sell_px=100, sell_qty=3000)

            # Send Bkt order
            print(f'Sending Bkt order pairs...')
            payload_bkt_SL = {"method": "PlaceOrder", "base": BASE_ID, "counter": COUNTER_ID, "price": 98, 'quantity': -1000,
                              "type": "stop_order", "tag": 8888, 'tonce': tonce,
                              "params": {"trigger_price": 98}}
            tonce_bkt_SL = tonce;
            tonce += 1;
            print(f'Request(3): {payload_bkt_SL}')
            await ws3.send(json.dumps(payload_bkt_SL))

            payload_bkt_TP = {"method": "PlaceOrder", "base": BASE_ID, "counter": COUNTER_ID, "price": 105, 'quantity': -1000,
                              "tag": 8888, 'tonce': tonce}
            tonce_bkt_TP = tonce;
            tonce += 1;
            print(f'Request(3): {payload_bkt_TP}')
            await ws3.send(json.dumps(payload_bkt_TP))
            await asyncio.sleep(2)

            # Cancel the limit TP and place big limit SELL to reduce available balance
            print(f'Cancel the limit TP and place a big limit SELL...')
            payload_cxl_TP = {"method": "CancelOrder", 'tonce': tonce_bkt_TP}
            print(f'Request(3): {payload_cxl_TP}')
            await ws3.send(json.dumps(payload_cxl_TP));
            payload_big_SELL = {"method": "PlaceOrder", "base": BASE_ID, "counter": COUNTER_ID, "price": 105, 'quantity': -99500,
                                "tag": 7777, 'tonce': tonce};
            tonce += 1
            print(f'Request(3): {payload_big_SELL}')
            await ws3.send(json.dumps(payload_big_SELL));
            payload_get_balance = {"tag": 8888, "method": "GetBalances"}
            print(f'Request(3): {payload_get_balance}')
            await ws3.send(json.dumps(payload_get_balance));
            await asyncio.sleep(1)

            # Execute a trade
            print(f'Execute a trade ...')
            payload_exec = {"method": "PlaceOrder", "base": BASE_ID, "counter": COUNTER_ID, "price": 98, 'quantity': -3000,
                            "tag": 999, 'tonce': tonce};
            tonce += 1;
            print(f'Request(2): {payload_exec}')
            await ws2.send(json.dumps(payload_exec))

            # Get Balance
            payload_get_balance = {"tag": 8888, "method": "GetBalances"}
            print(f'Request(3): {payload_get_balance}')
            await ws3.send(json.dumps(payload_get_balance));
            await asyncio.sleep(1)

            bid, ask = await get_order_book()
            assert (len(bid) == 0)
            assert (ask[0] == (98, -300))

            print(f'Receiving remaining notices...')
            while True:
                sys.stdout.flush();
                await asyncio.sleep(1)

        except Exception as error:
            err_msg = 'Error: ' + str(time.time()) + ' ' + repr(error)
            print(err_msg)

async def get_order_book():
    #global all_replies1, all_replies2, all_replies3
    global all_replies_map
    payload_get_orders = {"tag": 123, "method": "GetOrders"}
    for user_id in USER_IDS:
        await send_order(user_id, payload_get_orders)
    await asyncio.sleep(3)

    bid, ask = {}, {}
    for replies in [all_replies1, all_replies2, all_replies3]:
        msg = replies.pop()
        # print(msg)
        for o in msg['orders']:
            if o['quantity'] > 0:
                bid[o['price']] = bid.get(o['price'], 0) + o['quantity']
            else:
                ask[o['price']] = ask.get(o['price'], 0) + o['quantity']
    bid = sorted(bid.items())
    ask = sorted(ask.items())
    print('Orderbook:')
    print(f'  BID:{bid}')
    print(f'  ASK:{ask}')
    return bid, ask

if __name__ == '__main__':
    global sock
    #sock = ut.init_engine(logging.INFO, first_time=True)
    #sock = ut.init_engine(logging.INFO, host=HOST, first_time=False)
    #wipe_up(sock)
    loop = asyncio.get_event_loop()

    global tonce, ws_map, bAuth_map, all_replies_map, args_map, listen_map
    ws_map, bAuth_map, all_replies_map, args_map, listen_map, order_map = {}, {}, {}, {}, {}, {}
    tonce = int(time.time()) * 1000

    # Setup async tasks for login and listening to messages
    payload_place_sell1  = {'side':'SELL', "price":   int(7800*M), 'quantity':    1000,'order_action':'NEW',  'method':'PlaceOrder', 'client_order_id': tonce};      tonce += 1;
    payload_place_sell2  = {'side':'SELL', "price": int(0.0001*M), 'quantity':   1500,'order_action':'NEW',  'method':'PlaceOrder', 'client_order_id': tonce};      tonce += 1;
    payload_place_sell3  = {'side':'SELL', "price": 30, 'quantity':    999999980,'order_action':'NEW',  'method':'PlaceOrder', 'client_order_id': tonce};      tonce += 1;

    payload_place_buy1  = {'side':'BUY' , "price":         160, 'quantity':       720, 'order_action':'NEW', 'method':'PlaceOrder', 'client_order_id': tonce};      tonce += 1;
    payload_place_buy2  = {'side':'BUY' , "price": 2000000, 'quantity': 1600, 'order_action':'NEW', 'method':'PlaceOrder', 'client_order_id': tonce};      tonce += 1;
    #payload_place_buy3  = {'side':'BUY' , "price": 10, 'quantity': 5, 'order_action':'NEW', 'method':'PlaceOrder', 'client_order_id': tonce};      tonce += 1;

    payload_reconnect = { 'method' : 'ReconnectEngine'}

    #loop.create_task(test(HOST, payload_reconnect))
    #loop.create_task(test(HOST, PORT2, payload_place_sell1))
    loop.create_task(test(HOST, PORT5, payload_place_sell2))
    #loop.create_task(test(HOST, PORT5, payload_place_buy2))
    #loop.create_task(test(HOST, payload_place_buy1))
    # loop.create_task(test(HOST, payload_place_buy2))
    #loop.create_task(test(HOST, payload_place_sell3))
    #loop.create_task(test_stress(nUsers, 0.1))
    for uid in USER_IDS:
        order_map[uid] = deque(maxlen=10)
        loop.create_task(get_reply_notice(user_id=uid, sleep_s=0.001, bPrint=True))
        #break   # listen to 1 user for testing

    loop.run_forever()
