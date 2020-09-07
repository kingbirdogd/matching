#!/usr/bin/env bash
SCRIPT=`realpath $0`
export APPDIR=`dirname $SCRIPT`
echo "Pulsar host is : ${PLSR_URL}"
echo "REST URL    is : ${REST_URL}"

PAIR=$1

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
export LOG_LOCATION=${APPDIR}/log/$(date +%Y%m%d_%H%M%S)
echo ${LOG_LOCATION} > ${APPDIR}/LOG_LOCATION
export LD_LIBRARY_PATH=${BASEDIR}/usr/local/lib64:${BASEDIR}/usr/local/lib:${BASEDIR}/usr/local:$LD_LIBRARY_PATH

mkdir -p ${LOG_LOCATION}
ln -snf ${LOG_LOCATION} ${APPDIR}/log/clog

$APPDIR/me_server $APPDIR/$PAIR.json >> ${LOG_LOCATION}/me_server.out.log 2>> ${LOG_LOCATION}/me_server.err.log &
chmod 775 $APPDIR/start_proxy_$PAIR.sh
$APPDIR/start_proxy_$PAIR.sh

sleep 6
cmd="curl --verbose -X POST http://${REST_URL}/workingorder/recover/$PAIR"
echo "Recovering orders: $cmd" 2>&1 >> ${LOG_LOCATION}/me_server.err.log
$cmd &> ${LOG_LOCATION}/RECOVER_ORDERS.log

tail -f /dev/null