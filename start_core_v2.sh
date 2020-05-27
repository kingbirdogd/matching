#!/usr/bin/env bash

BASEDIR=/home/docker
CORE_BASE=${BASEDIR}/targets/coinflex_v2_core
CORE_LOCATION=${CORE_BASE}
USR_DIR=$BASEDIR/usr/local
LOG_LOCATION=${CORE_BASE}/log
export LD_LIBRARY_PATH=${USR_DIR}/lib64:${USR_DIR}/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=${BASEDIR}:$LD_LIBRARY_PATH
#source ${BASEDIR}/local/core_v2.properties

mkdir -p ${LOG_LOCATION}

${CORE_LOCATION}/xpubxsub 14001 14002 >> ${LOG_LOCATION}/xpubxsub.out.log 2>> ${LOG_LOCATION}/xpubxsub.err.log &

${CORE_LOCATION}/test_zmq_matching_server 13301 13302 13303 >> ${LOG_LOCATION}/server.out.log 2>> ${LOG_LOCATION}/server.err.log &
#${CORE_LOCATION}/bridge -v localhost >> ${LOG_LOCATION}/bridge.out.log 2>> ${LOG_LOCATION}/bridge.err.log &
#${CORE_LOCATION}/scribe -v localhost amqp://${AMQP_HOST}:5672/ rks rks >> ${LOG_LOCATION}/scribe.out.log 2>> ${LOG_LOCATION}/scribe.err.log &
${CORE_LOCATION}/proxy_lws -M 13301 -P 8081 --skipAuth --oneQueue -v localhost >> ${LOG_LOCATION}/proxy_lws1.out.log 2>> ${LOG_LOCATION}/proxy_lws1.err.log &
${CORE_LOCATION}/proxy_lws -M 13302 -P 8082 --skipAuth --oneQueue -v localhost >> ${LOG_LOCATION}/proxy_lws2.out.log 2>> ${LOG_LOCATION}/proxy_lws2.err.log &
${CORE_LOCATION}/proxy_lws -M 13303 -P 8083 --skipAuth --oneQueue -v localhost >> ${LOG_LOCATION}/proxy_lws3.out.log 2>> ${LOG_LOCATION}/proxy_lws3.err.log &

tail -f /dev/null

