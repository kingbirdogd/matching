#!/usr/bin/env bash
SCRIPT=`realpath $0`
APPDIR=`dirname $SCRIPT`
echo "Pulsar host is : ${PLSR_URL}"
echo "Pulsar host is : ${REST_URL}"

PAIR=$1

python3 $APPDIR/config_parser/gen_config.py ${REST_URL} $PAIR $APPDIR/$PAIR.json
