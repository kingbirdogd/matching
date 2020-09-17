#!/usr/bin/env bash
SCRIPT=`realpath $0`
export APPDIR=`dirname $SCRIPT`
echo "Pulsar host is : ${PLSR_URL}"
echo "REST URL    is : ${REST_URL}"

PAIR=$1

if [ $PAIR = "BCH-USD" ]; then
  $APPDIR/start_core_v2_BCH.sh
else

python3 $APPDIR/config_parser/gen_config.py ${REST_URL} $PAIR $APPDIR/$PAIR.json
python3 $APPDIR/config_parser/gen_proxy_script.py $APPDIR/$PAIR.json $APPDIR/start_proxy_$PAIR.sh
python3 $APPDIR/config_parser/gen_auction_cfg.py  $APPDIR/$PAIR.json $APPDIR/run_auction.json

if [ -f /etc/crontab ]; then
  cat $APPDIR/logrotate.cron   >> /etc/crontab
  cat $APPDIR/run_auction.cron >> /etc/crontab
  crond
  crontab /etc/crontab
fi

BASEDIR=/home/docker
export CORE_LOCATION=${APPDIR}
export DT=$(date +%Y%m%d_%H%M%S)
export LOG_LOCATION=${APPDIR}/log/$DT
echo ${LOG_LOCATION} > ${APPDIR}/LOG_LOCATION
export LD_LIBRARY_PATH=${BASEDIR}/usr/local/lib64:${BASEDIR}/usr/local/lib:${BASEDIR}/usr/local:$LD_LIBRARY_PATH

mkdir -p ${LOG_LOCATION}
cd ${APPDIR}/log
ln -snf ${DT} clog
cd -

$APPDIR/me_server $APPDIR/$PAIR.json >> ${LOG_LOCATION}/me_server.out.log 2>> ${LOG_LOCATION}/me_server.err.log &
chmod 775 $APPDIR/start_proxy_$PAIR.sh
$APPDIR/start_proxy_$PAIR.sh

#sleep 6
N=`grep market_id /app/$PAIR.json | wc -l`
M=`grep connected /app/log/clog/pulsar_proxy*.err.log | wc -l`
echo "$(date +%Y%m%d_%H:%M:%S) Waiting for connection to Pulsar... Connected = $M/$N"

while [ $N -ne $M ]; do
  echo "$(date +%Y%m%d_%H:%M:%S) Waiting for connection to Pulsar... Connected = $M/$N"
  sleep 1
  N=`grep market_id /app/$PAIR.json | wc -l`
  M=`grep connected /app/log/clog/pulsar_proxy*.err.log | wc -l`
done

echo "All pulsar_proxy connected to Pulsar"

cmd="curl --verbose -X POST http://${REST_URL}/workingorder/recover/$PAIR"
echo "Recovering orders: $cmd" 2>&1 >> ${LOG_LOCATION}/me_server.err.log
$cmd &> ${LOG_LOCATION}/RECOVER_ORDERS.log

tail -f /dev/null
fi