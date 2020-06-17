#~/bin/bash

MARKETPORT=34675
FACTOR=100000000
ACCOUNTID=1234567
MARKETID=2001031000000
QTY=2000
PX=10  # ARBITRARILY BIG
SIDE=BUY

# ==== Pause Receiving Orders ====
echo "Pause Receiving Orders"
sudo kill -s SIGUSR1 `pgrep -f "pulsar_proxy.*ORDER-IN-$MARKETID"`

./auction_tcp_matching_client 127.0.0.1 $MARKETPORT $FACTOR $ACCOUNTID $MARKETID $QTY $PX $SIDE

sleep 6

# ==== Resume Receiving Orders ====
echo "Resume Receiving Orders"
sudo kill -s SIGUSR1 `pgrep -f "pulsar_proxy.*ORDER-IN-$MARKETID"`
