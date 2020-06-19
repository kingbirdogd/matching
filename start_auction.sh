#~/bin/bash

MARKETPORT=34675
PXFACTOR=100000000
QTYFACTOR=100000000
ACCOUNTID=1234567
MARKETID=2001031000000
QTY=2000
PX=10  # ARBITRARILY BIG INTEGER
SIDE=BUY
BUY_UPPER_BAND=1000000
SELL_LOWER_BAND=-1000000

# ==== Pause Receiving Orders ====
echo "Pause Receiving Orders"
sudo kill -s SIGUSR1 `pgrep -f "pulsar_proxy.*ORDER-IN-$MARKETID"`

./auction_tcp_matching_client 127.0.0.1 $MARKETPORT $QTYFACTOR $PXFACTOR $ACCOUNTID $MARKETID $QTY $PX $SIDE $BUY_UPPER_BAND $SELL_LOWER_BAND

sleep 6

# ==== Resume Receiving Orders ====
echo "Resume Receiving Orders"
sudo kill -s SIGUSR1 `pgrep -f "pulsar_proxy.*ORDER-IN-$MARKETID"`
