import zmq
import struct

context = zmq.Context()

# setup zeromq socket
frontend = context.socket(zmq.SUB)
#frontend.connect("tcp://18.162.39.80:14001")
frontend.connect("tcp://127.0.0.1:36001")

# Subscribe on everything
frontend.setsockopt(zmq.SUBSCRIBE, b'')

while True:
    # Process all parts of the message
    message = frontend.recv()
    px   = struct.unpack('q', message[0:8])[0]
    qty  = struct.unpack('Q', message[8:16])[0]
    side = struct.unpack('Q', message[16:24])[0]
    market_id = struct.unpack('Q', message[24:32])[0]

    print(f'px={px}, qty={qty}, side={side}, market_id={market_id}')
    #print(message)
