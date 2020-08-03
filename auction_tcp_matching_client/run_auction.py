import requests
import os

url_stg  = '172.42.13.195:9988'
url_live = '172.41.11.60:9988'
market_id = '2001011000000'
url = url_stg
http_url = f'http://{url}/account/delivery/total/{market_id}'

if __name__ == '__main__' :
  res=requests.get(http_url)
  print(f'response: {res}')

  if res.status_code == 200:
    print('OK!')
    j = res.json()
    print(f'response: {j}')
    if len(j) > 0:
      netDelivery = j[0]['netDelivery']
      if abs(netDelivery) > 0:
        print(f'netDelivery: {netDelivery}')

        MARKETPORT = 34675
        QTYFACTOR  = 100000000
        PXFACTOR   = 100000000
        ACCOUNTID  = 9999999992
        MARKETID   = 4001031000000
        QTY  = abs(netDelivery)
        PX   = -10    if netDelivery < 0 else 10
        SIDE = 'SELL' if netDelivery < 0 else 'BUY'
        BUY_UPPER_BAND = 0.01
        SELL_LOWER_BAND = -0.01
        #${MARKETPORT} ${QTYFACTOR} ${PXFACTOR} ${ACCOUNTID} ${MARKETID} ${QTY} ${PX} ${SIDE} ${BUY_UPPER_BAND} ${SELL_LOWER_BAND}

        cmd = f'./start_auction.sh 127.0.0.1 {MARKETPORT} {QTYFACTOR} {PXFACTOR} {ACCOUNTID} {MARKETID} {QTY} {PX} {SIDE} {BUY_UPPER_BAND} {SELL_LOWER_BAND}'
        print(f'{cmd}')
        #cmd = '../start

