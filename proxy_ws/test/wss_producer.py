import zmq
import struct
import asyncio
import logging
import websockets
from websockets import WebSocketClientProtocol

context = zmq.Context()

# setup zeromq socket
frontend = context.socket(zmq.SUB)
#frontend.connect("tcp://18.162.39.80:14001")
frontend.connect("tcp://127.0.0.1:36001")

# Subscribe on everything
frontend.setsockopt(zmq.SUBSCRIBE, b'')
frontend.RCVTIMEO = 1

px=0; qty=0; side=0; market_id=0;

async def recv_zmq():
  global px, qty, side, market_id
  while True:
    # Process all parts of the message
    try:
      message = frontend.recv()
      px   = struct.unpack('q', message[0:8])[0]
      qty  = struct.unpack('Q', message[8:16])[0]
      side = struct.unpack('Q', message[16:24])[0]
      market_id = struct.unpack('Q', message[24:32])[0]

      print(f'px={px}, qty={qty}, side={side}, market_id={market_id}')
    except:
      pass
    finally:
      await asyncio.sleep(0.001)

async def produce(message:str, host:str, port:int) -> None:
  global px, qty, side, market_id
  async with websockets.connect(f'ws://{host}:{port}') as ws:
    while True:
      message = f'Update: px={px}, qty={qty}, side={side}, market_id={market_id}'
      await ws.send(message)
      #await ws.recv()
      await asyncio.sleep(0.1)

if __name__ == '__main__':
  loop = asyncio.get_event_loop()
  loop.create_task(recv_zmq())
  loop.run_until_complete(produce(message='hi', host='localhost', port=33333))
