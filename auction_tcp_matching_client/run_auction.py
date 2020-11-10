import requests
import json
import os
import sys

url_stg  = '172.42.13.195:9988'
url_live = '172.41.11.60:9988'
#market_id = '2001011000000'
#url = url_stg

if __name__ == '__main__' :
  print("=============================", flush=True)
  os.system('date +%Y-%m-%d_%H:%M:%S')
  jf = open(sys.argv[1])
  cfg = json.load(jf)
  ROOTDIR   = cfg['ROOT_DIR']
  url       = cfg['REST_URL']
  perp_id   = cfg['PERP_ID']
  market_id = cfg['REPO_ID']
  market_port = cfg['REPO_PORT']

  # ROOTDIR   = sys.argv[1]
  # url       = sys.argv[2]
  # perp_id   = sys.argv[3]
  # market_id = sys.argv[4]
  http_url  = f'http://{url}/account/delivery/total/{perp_id}'

  res=requests.get(http_url)
  print(f'response: {res}', flush=True)

  if res.status_code == 200:
    print('OK!', flush=True)
    j = res.json()
    print(f'response: {j}', flush=True)
    if len(j) > 0:
      netDelivery = j[0]['netDelivery']
      if abs(netDelivery) > 0:
        print(f'netDelivery: {netDelivery}', flush=True)

        MARKETPORT = market_port
        QTYFACTOR  = 100000000
        PXFACTOR   = 100000000
        ACCOUNTID  = 9999999992
        #MARKETID   = 4001031000000
        MARKETID   = market_id
        QTY  = abs(netDelivery)
        PX   = -10    if netDelivery < 0 else 10
        SIDE = 'SELL' if netDelivery < 0 else 'BUY'
        BUY_UPPER_BAND  = j[0]['upperPriceBound']
        SELL_LOWER_BAND = j[0]['lowerPriceBound']
        #BUY_UPPER_BAND = 0.01
        #SELL_LOWER_BAND = -0.01
        #${MARKETPORT} ${QTYFACTOR} ${PXFACTOR} ${ACCOUNTID} ${MARKETID} ${QTY} ${PX} ${SIDE} ${BUY_UPPER_BAND} ${SELL_LOWER_BAND}

        cmd = f'{ROOTDIR}/start_auction.sh 127.0.0.1 {MARKETPORT} {QTYFACTOR} {PXFACTOR} {ACCOUNTID} {MARKETID} {QTY} {PX} {SIDE} {BUY_UPPER_BAND} {SELL_LOWER_BAND}'
        print(f'{cmd}', flush=True)
        os.system(cmd)

