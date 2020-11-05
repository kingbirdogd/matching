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

  cpui = 1
  for i in j['instruments']:
    #print(f'{i["book_name"]} {Decimal(str(i["tick_sz"]))}')
    tsz = Decimal(str(i['tick_sz']))
    if i["book_name"] == "Futures":
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i['factor']} 127.0.0.1 34671 35671 127.0.0.1 {tsz} {i['maker_fees']} 127.0.0.1 34673 127.0.0.1 34672 a_bid_b_bid   a_ask_b_ask   add_bid_implier      add_ask_implier   >> ${{LOG_LOCATION}}/md_tcp1.out.log 2>> ${{LOG_LOCATION}}/md_tcp1.err.log &\nsleep 1\n"
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X 35671 -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar1.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar1.err.log &\nsleep 1\n"
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 34671 pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy1.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy1.err.log &\nsleep 1\n"
      cpui += 1
      outStr += "\n"
    elif i["book_name"] == "Perpetual":
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i['factor']} 127.0.0.1 34672 35672 127.0.0.1 {tsz} {i['maker_fees']} 127.0.0.1 34671 127.0.0.1 34673 a_bid_b_ask   a_ask_b_bid   minus_bid_implier    minus_ask_implier >> ${{LOG_LOCATION}}/md_tcp2.out.log 2>> ${{LOG_LOCATION}}/md_tcp2.err.log &\nsleep 1\n"
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X 35672 -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar2.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar2.err.log &\nsleep 1\n"
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 34672 pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy2.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy2.err.log &\nsleep 1\n"
      cpui += 1
      outStr += "\n"
    elif i["book_name"] == "Spread-Fut-Perp":
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i['factor']} 127.0.0.1 34673 35673 127.0.0.1 {tsz} {i['maker_fees']} 127.0.0.1 34671 127.0.0.1 34672 a_bid_b_ask   a_ask_b_bid   minus_bid_implier    minus_ask_implier >> ${{LOG_LOCATION}}/md_tcp3.out.log 2>> ${{LOG_LOCATION}}/md_tcp3.err.log &\nsleep 1\n"
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X 35673 -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar3.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar3.err.log &\nsleep 1\n"
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 34673 pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy3.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy3.err.log &\nsleep 1\n"
      cpui += 1
      outStr += "\n"
    elif i["book_name"] == "Spot":
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i['factor']} 127.0.0.1 34674 35674 127.0.0.1 {tsz} {i['maker_fees']} 127.0.0.1 34672 127.0.0.1 34675 a_bid_b_bid   a_ask_b_ask   repo_out_bid_implier repo_out_ask_implier  >> ${{LOG_LOCATION}}/md_tcp4.out.log 2>> ${{LOG_LOCATION}}/md_tcp4.err.log &\nsleep 1\n"
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X 35674 -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar4.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar4.err.log &\nsleep 1\n"
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 34674 pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy4.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy4.err.log &\nsleep 1\n"
      cpui += 1
      outStr += "\n"
    elif i["book_name"] == "Repo":
      outStr += f'taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i["factor"]} 127.0.0.1 34675 35675 127.0.0.1 {tsz} {i["maker_fees"]}       ""     0        ""     0 a_none_b_none a_none_b_none none                 none               >> ${{LOG_LOCATION}}/md_tcp5.out.log 2>> ${{LOG_LOCATION}}/md_tcp5.err.log &\nsleep 1\n'
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X 35675 -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar5.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar5.err.log &\nsleep 1\n"
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 34675 pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy5.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy5.err.log &\nsleep 1\n"
      cpui += 1
      outStr += "\n"
    elif i["book_name"] == "Index":
      outStr += f'taskset -c {cpui} ${{CORE_LOCATION}}/test_md_tcp_server {i["factor"]} 127.0.0.1 34676 35676 127.0.0.1 {tsz} {i["maker_fees"]}       ""     0        ""     0 a_none_b_none a_none_b_none none                 none               >> ${{LOG_LOCATION}}/md_tcp6.out.log 2>> ${{LOG_LOCATION}}/md_tcp6.err.log &\nsleep 1\n'
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/proxy_md_pulsar -s ${{CORE_LOCATION}}/md.fbs -B $PUB_TIME_MS -R pulsar://${{PLSR_URL}} -E persistent://CF-V2/ME-WS/MD-SNAPSHOT-{i['market_id']} -F persistent://CF-V2/ME-WS/MD-DIFF-{i['market_id']} -X 35676 -v --oneQueue --skipAuth localhost >> ${{LOG_LOCATION}}/proxy_md_pulsar6.out.log 2>> ${{LOG_LOCATION}}/proxy_md_pulsar6.err.log &\nsleep 1\n"
      cpui += 1
      outStr += f"taskset -c {cpui} ${{CORE_LOCATION}}/pulsar_proxy 127.0.0.1 34676 pulsar://${{PLSR_URL}} persistent://CF-V2/PRETRADE-ME/ORDER-IN-{i['market_id']} persistent://CF-V2/ME-POSTTRADE/ORDER-OUT-{i['market_id']} ${{CORE_LOCATION}}/msg.fbs >> ${{LOG_LOCATION}}/pulsar_proxy6.out.log 2>> ${{LOG_LOCATION}}/pulsar_proxy6.err.log &\nsleep 1\n"
      cpui += 1
      outStr += "\n"

  #outStr += 'tail -f /dev/null'
  print(outStr)
  with open(out_file, 'w') as outf:
    outf.write(outStr)