#!/usr/bin/env bash

# Book A
#   wss port for internal testing               :  8081
#   wss port for internal MD subscription (OKEx):  9081
#   zmq PUB port for snapshot                   : 18011
#   zmq PUB port for diff                       : 18012
#   zmq PULL port for receving ORDER            : 22011
#   zmq PUB  port for ORDER STATUS UPDATES      : 22012

# Book B
#   wss port for internal testing               :  8082
#   wss port for internal MD subscription (OKEx):  9082
#   zmq PUB port for snapshot                   : 18021
#   zmq PUB port for diff                       : 18022
#   zmq PULL port for receving ORDER            : 22021
#   zmq PUB  port for ORDER STATUS UPDATES      : 22022

# Book C (Spread A-B)
#   wss port for internal testing               :  8083
#   wss port for internal MD subscription (OKEx):  9083
#   zmq PUB port for snapshot                   : 18031
#   zmq PUB port for diff                       : 18032
#   zmq PULL port for receving ORDER            : 22031
#   zmq PUB  port for ORDER STATUS UPDATES      : 22032


BASEDIR=/home/docker
CORE_BASE=${BASEDIR}/targets/coinflex_v2_core
CORE_LOCATION=${CORE_BASE}
LOG_LOCATION=${CORE_BASE}/log
export LD_LIBRARY_PATH=${BASEDIR}/usr/local/lib64:${BASEDIR}/usr/local/lib:${BASEDIR}/usr/local:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=${CORE_LOCATION}/lib64:${CORE_LOCATION}/lib:$LD_LIBRARY_PATH
#source ${BASEDIR}/local/core_v2.properties

mkdir -p ${LOG_LOCATION}

#${CORE_LOCATION}/xpubxsub 14001 14002 >> ${LOG_LOCATION}/xpubxsub.out.log 2>> ${LOG_LOCATION}/xpubxsub.err.log &

# Matching server
${CORE_LOCATION}/test_tcp_matching_server 34671 34672 34673 >> ${LOG_LOCATION}/matching_server.out.log 2>> ${LOG_LOCATION}/matching_server.err.log &

# MD Implied Server
${CORE_LOCATION}/test_md_tcp_server 127.0.0.1 34671 127.0.0.1 34673 127.0.0.1 34672 a_bid_b_bid a_ask_b_ask add_bid_implier   add_ask_implier   35671 >> ${LOG_LOCATION}/md_tcp1.out.log 2>> ${LOG_LOCATION}/md_tcp1.err.log &
${CORE_LOCATION}/test_md_tcp_server 127.0.0.1 34672 127.0.0.1 34671 127.0.0.1 34673 a_bid_b_ask a_ask_b_bid minus_bid_implier minus_ask_implier 35672  >> ${LOG_LOCATION}/md_tcp2.out.log 2>> ${LOG_LOCATION}/md_tcp2.err.log &
${CORE_LOCATION}/test_md_tcp_server 127.0.0.1 34673 127.0.0.1 34671 127.0.0.1 34672 a_bid_b_ask a_ask_b_bid minus_bid_implier minus_ask_implier 35673  >> ${LOG_LOCATION}/md_tcp3.out.log 2>> ${LOG_LOCATION}/md_tcp3.err.log &

# MD Orderbook Server
${CORE_LOCATION}/proxy_md_lws -s ${CORE_LOCATION}/md.fbs -B 500 -E 18011 -F 18012 -G 9081 -X 35671 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md1.out.log 2>> ${LOG_LOCATION}/proxy_md1.err.log &
${CORE_LOCATION}/proxy_md_lws -s ${CORE_LOCATION}/md.fbs -B 500 -E 18021 -F 18022 -G 9082 -X 35672 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md2.out.log 2>> ${LOG_LOCATION}/proxy_md2.err.log &
${CORE_LOCATION}/proxy_md_lws -s ${CORE_LOCATION}/md.fbs -B 500 -E 18031 -F 18032 -G 9083 -X 35673 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md3.out.log 2>> ${LOG_LOCATION}/proxy_md3.err.log &

# ZMQ Proxy
${CORE_LOCATION}/zmq_proxy 127.0.0.1 34671 22011 22012 >> ${LOG_LOCATION}/zmq_proxy1.out.log 2>> ${LOG_LOCATION}/zmq_proxy1.err.log &
${CORE_LOCATION}/zmq_proxy 127.0.0.1 34672 22021 22022 >> ${LOG_LOCATION}/zmq_proxy2.out.log 2>> ${LOG_LOCATION}/zmq_proxy2.err.log &
${CORE_LOCATION}/zmq_proxy 127.0.0.1 34673 22031 22032 >> ${LOG_LOCATION}/zmq_proxy3.out.log 2>> ${LOG_LOCATION}/zmq_proxy3.err.log &

# WSS Proxy for testing
${CORE_LOCATION}/proxy_lws -M 34671 -P 8081 --skipAuth --oneQueue -v localhost >> ${LOG_LOCATION}/proxy_lws1.out.log 2>> ${LOG_LOCATION}/proxy_lws1.err.log &
${CORE_LOCATION}/proxy_lws -M 34672 -P 8082 --skipAuth --oneQueue -v localhost >> ${LOG_LOCATION}/proxy_lws2.out.log 2>> ${LOG_LOCATION}/proxy_lws2.err.log &
${CORE_LOCATION}/proxy_lws -M 34673 -P 8083 --skipAuth --oneQueue -v localhost >> ${LOG_LOCATION}/proxy_lws3.out.log 2>> ${LOG_LOCATION}/proxy_lws3.err.log &

tail -f /dev/null

