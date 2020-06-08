import pulsar


#client = pulsar.Client('pulsar://47.244.189.101:6650')
client = pulsar.Client('pulsar://172.21.11.79:6650')
#producer = client.create_producer('CF-V2/POSTTRADE-WS/ORDER-OUT-BTC-USD-SWAP-LIN')
#producer = client.create_producer('persistent://CF-V2/POSTTRADE-WS/ORDER-OUT-BTC-USD-SWAP-LIN')
#producer = client.create_producer('persistent://1234567890123456789012345678901234567890123456789012345678901234567890')
producer = client.create_producer(
                    'persistent://CF-V2/POSTTRADE-WS/TEST123',
                    block_if_queue_full=True,
                    batching_enabled=True,
                    batching_max_publish_delay_ms=10,
                    properties={
                        "producer-name": "test-producer-name",
                        "producer-id": "test-producer-id"
                    }
                )

for i in range(10):
  try:
    producer.send(('Hello-%d' % i).encode('utf-8'), None)
  except Exception as e:
    print("Failed to send message: %s", e)

producer.flush()
producer.close()

client.close()

