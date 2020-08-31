#!/usr/bin/env bash
SCRIPT=`realpath $0`
APPDIR=`dirname $SCRIPT`

if [ -f /etc/crontab ]; then
  cat $APPDIR/logrotate.cron   >> /etc/crontab
  cat $APPDIR/run_auction.cron >> /etc/crontab
  crond
  crontab /etc/crontab
fi

# aliyun-test: 172.21.21.221:6650
# aliyun-dev : 172.21.11.79:6650
#PLSR_URL="172.21.11.79:6650"

NODE_ID=5

if [ $# -eq 1 ]; then
  if [ $1 = "aliyun-test" ] ; then
    PLSR_URL="172.21.21.221:6650"
  elif [ $1 = "aliyun-dev" ] ; then
    PLSR_URL="172.21.11.79:6650"
  elif [ $1 = "local" ] ; then
    PLSR_URL="127.0.0.1:6650"
  fi
fi
echo "Pulsar host is : ${PLSR_URL}"

# Book A (Quaterly Futures)
#   wss port for internal testing               :  8081
#   wss port for internal MD subscription (OKEx):  9081
#   zmq PUB port for snapshot                   : 18011
#   zmq PUB port for diff                       : 18012
#   zmq PULL port for receving ORDER            : 22011
#   zmq PUSH port for ORDER STATUS UPDATES      : 22012

# Book B (Perp)
#   wss port for internal testing               :  8082
#   wss port for internal MD subscription (OKEx):  9082
#   zmq PUB port for snapshot                   : 18021
#   zmq PUB port for diff                       : 18022
#   zmq PULL port for receving ORDER            : 22021
#   zmq PUSH port for ORDER STATUS UPDATES      : 22022

# Book C (Spread A-B)
#   wss port for internal testing               :  8083
#   wss port for internal MD subscription (OKEx):  9083
#   zmq PUB port for snapshot                   : 18031
#   zmq PUB port for diff                       : 18032
#   zmq PULL port for receving ORDER            : 22031
#   zmq PUSH port for ORDER STATUS UPDATES      : 22032

# Book D (Spot)
#   wss port for internal testing               :  8084
#   wss port for internal MD subscription (OKEx):  9084
#   zmq PUB port for snapshot                   : 18041
#   zmq PUB port for diff                       : 18042
#   zmq PULL port for receving ORDER            : 22041
#   zmq PUSH port for ORDER STATUS UPDATES      : 22042

# Book E (Repo)
#   wss port for internal testing               :  8085
#   wss port for internal MD subscription (OKEx):  9085
#   zmq PUB port for snapshot                   : 18051
#   zmq PUB port for diff                       : 18052
#   zmq PULL port for receving ORDER            : 22051
#   zmq PUSH port for ORDER STATUS UPDATES      : 22052

BASEDIR=/home/docker
#CORE_BASE=${BASEDIR}/targets/coinflex_v2_core
CORE_LOCATION=${APPDIR}
LOG_LOCATION=${APPDIR}/log/$(date +%Y%m%d_%H%M%S)
echo ${LOG_LOCATION} > ${APPDIR}/LOG_LOCATION
export LD_LIBRARY_PATH=${BASEDIR}/usr/local/lib64:${BASEDIR}/usr/local/lib:${BASEDIR}/usr/local:$LD_LIBRARY_PATH
#export LD_LIBRARY_PATH=${CORE_LOCATION}/lib64:${CORE_LOCATION}/lib:$LD_LIBRARY_PATH
#source ${BASEDIR}/local/core_v2.properties

mkdir -p ${LOG_LOCATION}
ln -snf ${LOG_LOCATION} ${APPDIR}/log/clog

#${CORE_LOCATION}/xpubxsub 14001 14002 >> ${LOG_LOCATION}/xpubxsub.out.log 2>> ${LOG_LOCATION}/xpubxsub.err.log &

# Matching server
${CORE_LOCATION}/test_tcp_matching_server ${NODE_ID} 100000000 34671 34672 34673 34674 34675 34676 0.001 0.001 0.1 0.001 0.00005 1 2 2>&1 | tee -a ${LOG_LOCATION}/matching_server.err.log & # >> ${LOG_LOCATION}/matching_server.out.log &
sleep 2

# MD Implied Server
${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 34671 127.0.0.1 34673 127.0.0.1 34672 a_bid_b_bid   a_ask_b_ask   add_bid_implier      add_ask_implier      35671 127.0.0.1 0.001   0 >> ${LOG_LOCATION}/md_tcp1.out.log 2>> ${LOG_LOCATION}/md_tcp1.err.log &
sleep 1
${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 34672 127.0.0.1 34671 127.0.0.1 34673 a_bid_b_ask   a_ask_b_bid   minus_bid_implier    minus_ask_implier    35672 127.0.0.1 0.001   0 >> ${LOG_LOCATION}/md_tcp2.out.log 2>> ${LOG_LOCATION}/md_tcp2.err.log &
sleep 1
${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 34673 127.0.0.1 34671 127.0.0.1 34672 a_bid_b_ask   a_ask_b_bid   minus_bid_implier    minus_ask_implier    35673 127.0.0.1 0.1     2 >> ${LOG_LOCATION}/md_tcp3.out.log 2>> ${LOG_LOCATION}/md_tcp3.err.log &
sleep 1
${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 34674 127.0.0.1 34672 127.0.0.1 34675 a_bid_b_bid   a_ask_b_ask   repo_out_bid_implier repo_out_ask_implier 35674 127.0.0.1 0.001   0 >> ${LOG_LOCATION}/md_tcp4.out.log 2>> ${LOG_LOCATION}/md_tcp4.err.log &
sleep 1
${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 34675        ""     0        ""     0 a_none_b_none a_none_b_none none                 none                 35675 127.0.0.1 0.00005 0 >> ${LOG_LOCATION}/md_tcp5.out.log 2>> ${LOG_LOCATION}/md_tcp5.err.log &
sleep 1
#${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 34676        ""     0        ""     0 a_none_b_none a_none_b_none none                 none                 35676 127.0.0.1 0.005   0 >> ${LOG_LOCATION}/md_tcp6.out.log 2>> ${LOG_LOCATION}/md_tcp6.err.log &
#sleep 1

# MD Orderbook Server
PUB_TIME_MS=50
LINK_USD_QP=10001021200925
${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-${LINK_USD_QP}   -F persistent://CF-V2/ME-WS/MD-DIFF-${LINK_USD_QP}   -G 7081 -X 35671 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar1.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar1.err.log &
sleep 1
LINK_USD_SWAP=10001011000000
${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-${LINK_USD_SWAP} -F persistent://CF-V2/ME-WS/MD-DIFF-${LINK_USD_SWAP} -G 7082 -X 35672 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar2.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar2.err.log &
sleep 1
LINK_USD_SPR=10001051200925
${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-${LINK_USD_SPR}  -F persistent://CF-V2/ME-WS/MD-DIFF-${LINK_USD_SPR}  -G 7083 -X 35673 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar3.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar3.err.log &
sleep 1
LINK_USD=10001000000000
${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-${LINK_USD}      -F persistent://CF-V2/ME-WS/MD-DIFF-${LINK_USD}      -G 7084 -X 35674 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar4.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar4.err.log &
sleep 1
LINK_USD_REPO=10001031000000
${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-${LINK_USD_REPO} -F persistent://CF-V2/ME-WS/MD-DIFF-${LINK_USD_REPO} -G 7085 -X 35675 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar5.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar5.err.log &
sleep 1
#${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-3001000000000 -F persistent://CF-V2/ME-WS/MD-DIFF-3001000000000 -G 7086 -X 35676 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar6.out.log & #2>> ${LOG_LOCATION}/proxy_md_pulsar5.err.log &
#sleep 1

# Pulsar Proxy
${CORE_LOCATION}/pulsar_proxy 127.0.0.1 34671 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-${LINK_USD_QP}   persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-${LINK_USD_QP}   ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy1.out.log 2>> ${LOG_LOCATION}/pulsar_proxy1.err.log &
sleep 1
${CORE_LOCATION}/pulsar_proxy 127.0.0.1 34672 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-${LINK_USD_SWAP} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-${LINK_USD_SWAP} ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy2.out.log 2>> ${LOG_LOCATION}/pulsar_proxy2.err.log &
sleep 1
${CORE_LOCATION}/pulsar_proxy 127.0.0.1 34673 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-${LINK_USD_SPR}  persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-${LINK_USD_SPR}  ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy3.out.log 2>> ${LOG_LOCATION}/pulsar_proxy3.err.log &
sleep 1
${CORE_LOCATION}/pulsar_proxy 127.0.0.1 34674 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-${LINK_USD}      persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-${LINK_USD}      ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy4.out.log 2>> ${LOG_LOCATION}/pulsar_proxy4.err.log &
sleep 1
${CORE_LOCATION}/pulsar_proxy 127.0.0.1 34675 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-${LINK_USD_REPO} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-${LINK_USD_REPO} ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy5.out.log 2>> ${LOG_LOCATION}/pulsar_proxy5.err.log &
sleep 1
#${CORE_LOCATION}/pulsar_proxy 127.0.0.1 34676 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-3001000000000 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-3001000000000 >> ${LOG_LOCATION}/pulsar_proxy6.out.log 2>> ${LOG_LOCATION}/pulsar_proxy6.err.log &
#sleep 1

## MD Orderbook Server
#${CORE_LOCATION}/proxy_md_lws -s ${CORE_LOCATION}/md.fbs -B 500 -E 18011 -F 18012 -G 9081 -X 35671 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md1.out.log & #2>> ${LOG_LOCATION}/proxy_md1.err.log &
#sleep 1
#${CORE_LOCATION}/proxy_md_lws -s ${CORE_LOCATION}/md.fbs -B 500 -E 18021 -F 18022 -G 9082 -X 35672 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md2.out.log & #2>> ${LOG_LOCATION}/proxy_md2.err.log &
#sleep 1
#${CORE_LOCATION}/proxy_md_lws -s ${CORE_LOCATION}/md.fbs -B 500 -E 18031 -F 18032 -G 9083 -X 35673 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md3.out.log & #2>> ${LOG_LOCATION}/proxy_md3.err.log &
#sleep 1
#${CORE_LOCATION}/proxy_md_lws -s ${CORE_LOCATION}/md.fbs -B 500 -E 18041 -F 18042 -G 9084 -X 35674 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md4.out.log & #2>> ${LOG_LOCATION}/proxy_md4.err.log &
#sleep 1
#${CORE_LOCATION}/proxy_md_lws -s ${CORE_LOCATION}/md.fbs -B 500 -E 18051 -F 18052 -G 9085 -X 35675 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md5.out.log & #2>> ${LOG_LOCATION}/proxy_md5.err.log &
#sleep 1
#
## ZMQ Proxy
#${CORE_LOCATION}/zmq_proxy 127.0.0.1 34671 22011 22012 >> ${LOG_LOCATION}/zmq_proxy1.out.log 2>> ${LOG_LOCATION}/zmq_proxy1.err.log &
#sleep 1
#${CORE_LOCATION}/zmq_proxy 127.0.0.1 34672 22021 22022 >> ${LOG_LOCATION}/zmq_proxy2.out.log 2>> ${LOG_LOCATION}/zmq_proxy2.err.log &
#sleep 1
#${CORE_LOCATION}/zmq_proxy 127.0.0.1 34673 22031 22032 >> ${LOG_LOCATION}/zmq_proxy3.out.log 2>> ${LOG_LOCATION}/zmq_proxy3.err.log &
#sleep 1
#${CORE_LOCATION}/zmq_proxy 127.0.0.1 34674 22041 22042 >> ${LOG_LOCATION}/zmq_proxy4.out.log 2>> ${LOG_LOCATION}/zmq_proxy4.err.log &
#sleep 1
#${CORE_LOCATION}/zmq_proxy 127.0.0.1 34675 22051 22052 >> ${LOG_LOCATION}/zmq_proxy5.out.log 2>> ${LOG_LOCATION}/zmq_proxy5.err.log &
#sleep 1
#
## WSS Proxy for testing
#${CORE_LOCATION}/proxy_lws -M 34671 -P 8081 --skipAuth --oneQueue -v localhost >> ${LOG_LOCATION}/proxy_lws1.out.log 2>> ${LOG_LOCATION}/proxy_lws1.err.log &
#sleep 1
#${CORE_LOCATION}/proxy_lws -M 34672 -P 8082 --skipAuth --oneQueue -v localhost >> ${LOG_LOCATION}/proxy_lws2.out.log 2>> ${LOG_LOCATION}/proxy_lws2.err.log &
#sleep 1
#${CORE_LOCATION}/proxy_lws -M 34673 -P 8083 --skipAuth --oneQueue -v localhost >> ${LOG_LOCATION}/proxy_lws3.out.log 2>> ${LOG_LOCATION}/proxy_lws3.err.log &
#sleep 1
#${CORE_LOCATION}/proxy_lws -M 34674 -P 8084 --skipAuth --oneQueue -v localhost >> ${LOG_LOCATION}/proxy_lws4.out.log 2>> ${LOG_LOCATION}/proxy_lws4.err.log &
#sleep 1
#${CORE_LOCATION}/proxy_lws -M 34675 -P 8085 --skipAuth --oneQueue -v localhost >> ${LOG_LOCATION}/proxy_lws5.out.log 2>> ${LOG_LOCATION}/proxy_lws5.err.log &
#sleep 1

tail -f /dev/null

