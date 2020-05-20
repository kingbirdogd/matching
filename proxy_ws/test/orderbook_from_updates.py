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
import logging
import sys
from _collections import deque
from datetime import datetime

HOST  = 'localhost'
PORT  = 8080
#HOST  = 'ironmanapi3.coinflex.com'
#PORT  = 0

USER_IDS = [1]
nUsers   = len(USER_IDS)
updates  = {}

class Args:
    def __init__(self, user_id, port):
        if port == 0:
            self.url = f"wss://{HOST}"
        else:
            self.port = ':' + str(port)
            self.url  = f"ws://{HOST}{self.port}"
        self.id   = user_id          # this is core ID for your CoinFLEX account
        self.tag  = 1000 + user_id   # user_id = 1  ==> tag = 1001
        self.cookie ='YptI5VM7gq5XCEolVumsqDG412s='
        self.phrase = ''  # this is password for your CoinFLEX account

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
            #print(f'Update({user_id}): {msg}')
            if msg['remain_quantity'] == 0:
              if msg['order_id'] in updates.keys():
                del updates[msg['order_id']]
            else:
              updates[msg['order_id']] = msg
            get_order_book()
        if bPrint:
            sys.stdout.flush()

async def reconnect(sleep_s):
    global ws_map, listen_map, tonce

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
        reconnect_payload = {'method':'ReconnectEngine'}
        await send_order(USER_IDS[0], reconnect_payload)
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
    try:
        await auth(user_id)
        while True:
            await asyncio.sleep(0.1)
            if bAuth_map[user_id]:
                print(f'User {user_id} login successfully.')
                break
    except Exception as error:
        err_msg = 'Error: ' + str(time.time()) + ' ' + repr(error)
        print(err_msg)
        sys.exit(-1)

async def send_order(user_id, payload):
    global ws_map
    print(f'User {user_id} sending: {payload}')
    ws = ws_map[user_id]
    await ws.send(json.dumps(payload))

def get_order_book():
    global updates
    bid, ask = {}, {}
    buy_stop, sell_stop = [], []
    wQ = 0
    for o in updates.values():
        #print(o)
        if o['display_quantity'] > 0:
            if o['side'][0:3] == 'BUY':
                if o['price'] != 0:
                  bid[o['price']] = bid.get(o['price'], 0) + o['display_quantity']
                  wQ = max(wQ, len(str(bid[o['price']])))
                elif o['side'] == 'BUY_STOP':
                  #bid_stop[o['buy_stop_trigger_price']] = bid_stop.get(o['buy_stop_trigger_price'], [])
                  buy_stop.append(o)
            elif o['side'][0:4] == 'SELL':
                if o['price'] != 0:
                  ask[o['price']] = ask.get(o['price'], 0) + o['display_quantity']
                  wQ = max(wQ, len(str(ask[o['price']])))
                elif o['side'] == 'SELL_STOP':
                  #ask_stop[o['sell_stop_trigger_price']] = ask_stop.get(o['sell_stop_trigger_price'], [])
                  sell_stop.append(o)

    bid = sorted(bid.items(), key=lambda x:x[0], reverse=True)
    ask = sorted(ask.items(), key=lambda x:x[0])
    wP = len(str(ask[-1][0])) if len(ask) > 0 else (len(str(bid[0][0])) if len(bid) >0 else 0)
    wP = max(4, wP)
    s = ' '
    print('{dt}  (wQ={wQ} wP={wP})'.format(dt=datetime.now(), wP=wP, wQ=wQ))
    print('{BidQ:>{wQ}s} {BidP:>{wP}s} {AskP:>{wP}s} {AskQ:>{wQ}s}'.format(BidQ='BidQ', BidP='BidP', AskP='AskP', AskQ='AskQ', wP=wP, wQ=wQ))
    for i in range(0, max(len(bid), len(ask))):
        if i < len(bid):
            print(f'{bid[i][1]:{wQ}} {bid[i][0]:{wP}} ', end='')
        else:
            print(f'{s:{wQ}s} {s:{wP}s} ', end='')
        if i < len(ask):
            print(f'{ask[i][0]:{wP}} {ask[i][1]:{wQ}}')
        else:
            print(f'{s:{wP}s} {s:{wQ}s} ')

    # Display Stop orders
    print('BUY STOP orders: (trigger limit quantity)')
    buy_stop = sorted(buy_stop, key=lambda x:(x['buy_stop_trigger_price'], x['buy_stop_limited_price'], x['display_quantity'],), reverse=True)
    for o in buy_stop:
      print(f'  ({o["buy_stop_trigger_price"]} {o["buy_stop_limited_price"]} {o["display_quantity"]})')

    print('ASK STOP orders: (trigger limit quantity)')
    sell_stop = sorted(sell_stop, key=lambda x:(x['sell_stop_trigger_price'], x['sell_stop_limited_price'], x['display_quantity'],))
    for o in sell_stop:
      print(f'  ({o["sell_stop_trigger_price"]} {o["sell_stop_limited_price"]} {o["display_quantity"]})')

    return bid, ask

if __name__ == '__main__':
    loop = asyncio.get_event_loop()

    global tonce, ws_map, bAuth_map, all_replies_map, args_map, listen_map
    ws_map, bAuth_map, all_replies_map, args_map, listen_map, order_map = {}, {}, {}, {}, {}, {}
    tonce = int(time.time()) * 1000

    # Setup async tasks for login and listening to messages
    loop.create_task(reconnect(0.01))
    for uid in USER_IDS:
        order_map[uid] = deque(maxlen=10)
        loop.create_task(get_reply_notice(user_id=uid, sleep_s=0.001, bPrint=True))

    loop.run_forever()
