import requests
#import json
import simplejson as json
import os
import sys
from decimal import  Decimal

if __name__ == '__main__':
  print("=============================", flush=True)
  os.system('date +%Y-%m-%d_%H:%M:%S')
  jcfg     = open(sys.argv[1])
  out_file = sys.argv[2]
  outStr   = """#!/usr/bin/env bash
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
"""
  outStr += 'PUB_TIME_MS=50\nsleep 1\n'

  j = json.load(jcfg)

  portMap = {}
  for i in j['instruments']:
    portMap[i['market_code']] = i['port']

  cpui = 1
  for i in j['instruments']:
    #print(f'{i["book_name"]} {Decimal(str(i["tick_sz"]))}')
    tsz = Decimal(str(i['tick_sz']))
    md_port = i['port'] + 1000
    if i["book_name"] == "Futures":
      fees = i['impliers'][0]['ai_maker_fees'] if len(i['impliers']) > 0 else 0.0  # assume only 1 implier
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i['factor']} 127.0.0.1 {i['port']} {md_port} 127.0.0.1 {tsz} {fees} 127.0.0.1 {portMap[i['impliers'][0]['bi_leg1']]} 127.0.0.1 {portMap[i['impliers'][0]['bi_leg2']]} a_bid_b_bid   a_ask_b_ask   add_bid_implier      add_ask_implier   >> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X {md_port} -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 {i['port']} pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += "\n"
    elif i["book_name"] == "Perpetual":
      fees = i['impliers'][0]['ai_maker_fees'] if len(i['impliers']) > 0 else 0.0  # assume only 1 implier
      #outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i['factor']} 127.0.0.1 {i['port']} {md_port} 127.0.0.1 {tsz} {fees} 127.0.0.1 {portMap[i['impliers'][0]['bi_leg1']]} 127.0.0.1 {portMap[i['impliers'][0]['bi_leg2']]} a_bid_b_ask   a_ask_b_bid   minus_bid_implier    minus_ask_implier >> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.err.log &\nsleep 1\n"
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i['factor']} 127.0.0.1 {i['port']} {md_port} 127.0.0.1 {tsz} {fees}        \"\"     0        \"\"     0 a_none_b_none a_none_b_none none                 none >> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X {md_port} -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 {i['port']} pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += "\n"
    elif i["book_name"] == "Spread-Fut-Perp":
      fees = i['impliers'][0]['ai_maker_fees'] if len(i['impliers']) > 0 else 0.0  # assume only 1 implier
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i['factor']} 127.0.0.1 {i['port']} {md_port} 127.0.0.1 {tsz} {fees} 127.0.0.1 {portMap[i['impliers'][0]['bi_leg1']]} 127.0.0.1 {portMap[i['impliers'][0]['bi_leg2']]} a_bid_b_ask   a_ask_b_bid   minus_bid_implier    minus_ask_implier >> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X {md_port} -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 {i['port']} pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += "\n"
    elif i["book_name"] == "Spot":
      fees = i['impliers'][0]['ai_maker_fees'] if len(i['impliers']) > 0 else 0.0  # assume only 1 implier
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i['factor']} 127.0.0.1 {i['port']} {md_port} 127.0.0.1 {tsz} {fees} 127.0.0.1 {portMap[i['impliers'][0]['bi_leg1']]} 127.0.0.1 {portMap[i['impliers'][0]['bi_leg2']]} a_bid_b_bid   a_ask_b_ask   repo_out_bid_implier repo_out_ask_implier  >> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X {md_port} -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 {i['port']} pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += "\n"
    elif i["book_name"] == "Repo":
      fees = i['impliers'][0]['ai_maker_fees'] if len(i['impliers']) > 0 else 0.0  # assume only 1 implier
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i['factor']} 127.0.0.1 {i['port']} {md_port} 127.0.0.1 {tsz} {fees}       \"\"     0        \"\"     0 a_none_b_none a_none_b_none none                 none               >> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X {md_port} -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 {i['port']} pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += "\n"
    elif i["book_name"] == "Index":
      fees = i['impliers'][0]['ai_maker_fees'] if len(i['impliers']) > 0 else 0.0  # assume only 1 implier
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i['factor']} 127.0.0.1 {i['port']} {md_port} 127.0.0.1 {tsz} {fees}       \"\"     0        \"\"     0 a_none_b_none a_none_b_none none                 none               >> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/md_tcp_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X {md_port} -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 {i['port']} pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy_{i['market_code']}.err.log &\nsleep 1\n"
      cpui += 1; cpui %= 16
      outStr += "\n"

  #outStr += 'tail -f /dev/null'
  print(outStr)
  with open(out_file, 'w') as outf:
    outf.write(outStr)