import asyncio
import logging
import websockets
from websockets import WebSocketServerProtocol

logging.basicConfig(level=logging.INFO)

class Server:
  global px, qty, side
  clients = set()

  async def register(self, ws:WebSocketServerProtocol) -> None:
    self.clients.add(ws)
    logging.info(f'{ws.remote_address} connects.')

  async def unregister(self, ws:WebSocketServerProtocol) -> None:
    self.clients.remove(ws)
    logging.info(f'{ws.remote_address} disconnects.')

  async def send_to_clients(self, ws: WebSocketServerProtocol, message: str) -> None:
    if self.clients:
      await asyncio.wait([client.send(message) for client in self.clients if client != ws ])

  async def ws_handler(self, ws:WebSocketServerProtocol, uri:str) -> None:
    await self.register(ws)
    try:
      await self.distribute(ws)
    finally:
      await self.unregister(ws)

  async def distribute(self, ws: WebSocketServerProtocol) -> None:
    async for message in ws:
      await self.send_to_clients(ws, message)

if __name__ == '__main__':
  server = Server()
  start_server = websockets.serve(server.ws_handler, '127.0.0.1', 33333)
  loop = asyncio.get_event_loop()
  #loop.create_task(websockets.serve(server.ws_handler, '127.0.0.1', 33333))

  loop.run_until_complete(start_server)
  #loop.run_until_complete(recv_zmq)
  loop.run_forever()