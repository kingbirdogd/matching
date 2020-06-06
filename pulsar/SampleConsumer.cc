#include <iostream>
#include <pulsar/Client.h>
#include "lib/LogUtils.h"

DECLARE_LOG_OBJECT()

using namespace pulsar;

int main() {

    Client client("pulsar://localhost:6650");

    Consumer consumer;
    Result result = client.subscribe("persistent://prop/r1/ns1/my-topic", "consumer-1", consumer);
    if (result != ResultOk) {
        LOG_ERROR("Failed to subscribe: " << result);
        return -1;
    }

    Message msg;

    while (true) {
        consumer.receive(msg);
        LOG_INFO("Received: " << msg << "  with payload '" << msg.getDataAsString() << "'");
        LOG_INFO("Received: " << msg << "  with payload '" << msg.getData() << "'");

        consumer.acknowledge(msg);
    }

    client.close();
}
