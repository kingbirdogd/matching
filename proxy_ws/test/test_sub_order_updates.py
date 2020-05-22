import zmq

context = zmq.Context()

# setup zeromq socket
frontend = context.socket(zmq.SUB)
frontend.connect("tcp://18.162.39.80:14001")

# Subscribe on everything
frontend.setsockopt(zmq.SUBSCRIBE, b'')

while True:
    # Process all parts of the message
    message = frontend.recv()
    print(message)
