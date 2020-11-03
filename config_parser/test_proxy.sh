#!/usr/bin/env bash
# SCRIPT=`realpath $0`
# APPDIR=`dirname $SCRIPT`

# BASEDIR=/home/docker
# CORE_LOCATION=${APPDIR}
# LOG_LOCATION=${APPDIR}/log/$(date +%Y%m%d_%H%M%S)
# echo ${LOG_LOCATION} > ${APPDIR}/LOG_LOCATION
# export LD_LIBRARY_PATH=${BASEDIR}/usr/local/lib64:${BASEDIR}/usr/local/lib:${BASEDIR}/usr/local:$LD_LIBRARY_PATH
# 
# mkdir -p ${LOG_LOCATION}
# cd ${APPDIR}/log
# ln -snf ${LOG_LOCATION} clog
# cd -
# 
# echo "Pulsar host is : ${PLSR_URL}"
PUB_TIME_MS=50
sleep 1
taskset -c 1 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36671 37671 127.0.0.1 0.5 2             0             0 a_none_b_none a_none_b_none none                 none >> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.err.log &
sleep 1
taskset -c 2 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001011000000 -F persistent://CF-V2/ME-WS/MD-DIFF-2001011000000 -X 37671 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SWAP-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SWAP-LIN.err.log &
sleep 1
taskset -c 3 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36671 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001011000000 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001011000000 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SWAP-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SWAP-LIN.err.log &
sleep 1

taskset -c 4 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36672 37672 127.0.0.1 1 2 127.0.0.1 36675 127.0.0.1 36671 a_bid_b_ask   a_ask_b_bid   minus_bid_implier    minus_ask_implier >> ${LOG_LOCATION}/md_tcp_BTC-USD-SPR-210924P-LIN.out.log 2>> ${LOG_LOCATION}/md_tcp_BTC-USD-SPR-210924P-LIN.err.log &
sleep 1
taskset -c 5 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001041210924 -F persistent://CF-V2/ME-WS/MD-DIFF-2001041210924 -X 37672 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SPR-210924P-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SPR-210924P-LIN.err.log &
sleep 1
taskset -c 6 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36672 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001041210924 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001041210924 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SPR-210924P-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SPR-210924P-LIN.err.log &
sleep 1

taskset -c 7 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36673 37673 127.0.0.1 0.00005 2            0             0 a_none_b_none a_none_b_none none                 none               >> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.err.log &
sleep 1
taskset -c 8 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001031000000 -F persistent://CF-V2/ME-WS/MD-DIFF-2001031000000 -X 37673 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-REPO-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-REPO-LIN.err.log &
sleep 1
taskset -c 9 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36673 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001031000000 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001031000000 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-REPO-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-REPO-LIN.err.log &
sleep 1

taskset -c 10 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36674 37674 127.0.0.1 0.5 2 127.0.0.1 36681 127.0.0.1 36671 a_bid_b_bid   a_ask_b_ask   add_bid_implier      add_ask_implier   >> ${LOG_LOCATION}/md_tcp_BTC-USD-201225-LIN.out.log 2>> ${LOG_LOCATION}/md_tcp_BTC-USD-201225-LIN.err.log &
sleep 1
taskset -c 11 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001021201225 -F persistent://CF-V2/ME-WS/MD-DIFF-2001021201225 -X 37674 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-201225-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-201225-LIN.err.log &
sleep 1
taskset -c 12 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36674 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001021201225 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001021201225 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-201225-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-201225-LIN.err.log &
sleep 1

taskset -c 13 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36675 37675 127.0.0.1 0.5 2 127.0.0.1 36672 127.0.0.1 36671 a_bid_b_bid   a_ask_b_ask   add_bid_implier      add_ask_implier   >> ${LOG_LOCATION}/md_tcp_BTC-USD-210924-LIN.out.log 2>> ${LOG_LOCATION}/md_tcp_BTC-USD-210924-LIN.err.log &
sleep 1
taskset -c 14 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001021210924 -F persistent://CF-V2/ME-WS/MD-DIFF-2001021210924 -X 37675 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-210924-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-210924-LIN.err.log &
sleep 1
taskset -c 15 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36675 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001021210924 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001021210924 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-210924-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-210924-LIN.err.log &
sleep 1

taskset -c 16 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36676 37676 127.0.0.1 0.001 2 127.0.0.1 36678 127.0.0.1 36671 a_bid_b_bid   a_ask_b_ask   add_bid_implier      add_ask_implier   >> ${LOG_LOCATION}/md_tcp_BTC-USD-210326-LIN.out.log 2>> ${LOG_LOCATION}/md_tcp_BTC-USD-210326-LIN.err.log &
sleep 1
taskset -c 17 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001021210326 -F persistent://CF-V2/ME-WS/MD-DIFF-2001021210326 -X 37676 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-210326-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-210326-LIN.err.log &
sleep 1
taskset -c 18 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36676 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001021210326 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001021210326 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-210326-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-210326-LIN.err.log &
sleep 1

taskset -c 19 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36677 37677 127.0.0.1 0.1 2 127.0.0.1 36671 127.0.0.1 36673 a_bid_b_bid   a_ask_b_ask   repo_out_bid_implier repo_out_ask_implier  >> ${LOG_LOCATION}/md_tcp_BTC-USD.out.log 2>> ${LOG_LOCATION}/md_tcp_BTC-USD.err.log &
sleep 1
taskset -c 20 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001000000000 -F persistent://CF-V2/ME-WS/MD-DIFF-2001000000000 -X 37677 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD.err.log &
sleep 1
taskset -c 21 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36677 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001000000000 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001000000000 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD.err.log &
sleep 1

taskset -c 22 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36678 37678 127.0.0.1 1 2 127.0.0.1 36676 127.0.0.1 36671 a_bid_b_ask   a_ask_b_bid   minus_bid_implier    minus_ask_implier >> ${LOG_LOCATION}/md_tcp_BTC-USD-SPR-210326P-LIN.out.log 2>> ${LOG_LOCATION}/md_tcp_BTC-USD-SPR-210326P-LIN.err.log &
sleep 1
taskset -c 23 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001041210326 -F persistent://CF-V2/ME-WS/MD-DIFF-2001041210326 -X 37678 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SPR-210326P-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SPR-210326P-LIN.err.log &
sleep 1
taskset -c 24 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36678 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001041210326 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001041210326 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SPR-210326P-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SPR-210326P-LIN.err.log &
sleep 1

taskset -c 25 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36679 37679 127.0.0.1 1 2 127.0.0.1 36683 127.0.0.1 36671 a_bid_b_ask   a_ask_b_bid   minus_bid_implier    minus_ask_implier >> ${LOG_LOCATION}/md_tcp_BTC-USD-SPR-210625P-LIN.out.log 2>> ${LOG_LOCATION}/md_tcp_BTC-USD-SPR-210625P-LIN.err.log &
sleep 1
taskset -c 26 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001041210625 -F persistent://CF-V2/ME-WS/MD-DIFF-2001041210625 -X 37679 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SPR-210625P-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SPR-210625P-LIN.err.log &
sleep 1
taskset -c 27 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36679 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001041210625 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001041210625 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SPR-210625P-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SPR-210625P-LIN.err.log &
sleep 1

taskset -c 28 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36680 37680 127.0.0.1 0.5 2 127.0.0.1 36682 127.0.0.1 36671 a_bid_b_bid   a_ask_b_ask   add_bid_implier      add_ask_implier   >> ${LOG_LOCATION}/md_tcp_BTC-USD-211231-LIN.out.log 2>> ${LOG_LOCATION}/md_tcp_BTC-USD-211231-LIN.err.log &
sleep 1
taskset -c 29 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001021211231 -F persistent://CF-V2/ME-WS/MD-DIFF-2001021211231 -X 37680 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-211231-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-211231-LIN.err.log &
sleep 1
taskset -c 30 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36680 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001021211231 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001021211231 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-211231-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-211231-LIN.err.log &
sleep 1

taskset -c 31 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36681 37681 127.0.0.1 1.0 2 127.0.0.1 36674 127.0.0.1 36671 a_bid_b_ask   a_ask_b_bid   minus_bid_implier    minus_ask_implier >> ${LOG_LOCATION}/md_tcp_BTC-USD-SPR-201225P-LIN.out.log 2>> ${LOG_LOCATION}/md_tcp_BTC-USD-SPR-201225P-LIN.err.log &
sleep 1
taskset -c 32 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001051001225 -F persistent://CF-V2/ME-WS/MD-DIFF-2001051001225 -X 37681 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SPR-201225P-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SPR-201225P-LIN.err.log &
sleep 1
taskset -c 33 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36681 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001051001225 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001051001225 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SPR-201225P-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SPR-201225P-LIN.err.log &
sleep 1

taskset -c 34 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36682 37682 127.0.0.1 1 2 127.0.0.1 36680 127.0.0.1 36671 a_bid_b_ask   a_ask_b_bid   minus_bid_implier    minus_ask_implier >> ${LOG_LOCATION}/md_tcp_BTC-USD-SPR-211231P-LIN.out.log 2>> ${LOG_LOCATION}/md_tcp_BTC-USD-SPR-211231P-LIN.err.log &
sleep 1
taskset -c 35 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001041211231 -F persistent://CF-V2/ME-WS/MD-DIFF-2001041211231 -X 37682 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SPR-211231P-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-SPR-211231P-LIN.err.log &
sleep 1
taskset -c 36 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36682 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001041211231 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001041211231 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SPR-211231P-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-SPR-211231P-LIN.err.log &
sleep 1

taskset -c 37 ${CORE_LOCATION}/test_md_tcp_server 100000000 127.0.0.1 36683 37683 127.0.0.1 0.5 2 127.0.0.1 36679 127.0.0.1 36671 a_bid_b_bid   a_ask_b_ask   add_bid_implier      add_ask_implier   >> ${LOG_LOCATION}/md_tcp_BTC-USD-210625-LIN.out.log 2>> ${LOG_LOCATION}/md_tcp_BTC-USD-210625-LIN.err.log &
sleep 1
taskset -c 38 ${CORE_LOCATION}/proxy_md_pulsar -s ${CORE_LOCATION}/md.fbs -B $PUB_TIME_MS -R pulsar://${PLSR_URL} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-2001021210625 -F persistent://CF-V2/ME-WS/MD-DIFF-2001021210625 -X 37683 -v --oneQueue --skipAuth localhost >> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-210625-LIN.out.log 2>> ${LOG_LOCATION}/proxy_md_pulsar_BTC-USD-210625-LIN.err.log &
sleep 1
taskset -c 39 ${CORE_LOCATION}/pulsar_proxy 127.0.0.1 36683 pulsar://${PLSR_URL} persistent://CF-V2/PRETRADE-ME/ORDER-IN-2001021210625 persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-2001021210625 ${CORE_LOCATION}/msg.fbs >> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-210625-LIN.out.log 2>> ${LOG_LOCATION}/pulsar_proxy_BTC-USD-210625-LIN.err.log &
sleep 1

