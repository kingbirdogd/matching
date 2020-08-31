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

#$APPDIR/me_server $APPDIR/$PAIR.json
#$APPDIR/start_proxy_$PAIR.sh

tail -f /dev/null