import requests
#import json
import simplejson as json
import os
import sys
from decimal import  Decimal
import time

#json.encoder.FLOAT_REPR = lambda f: ("%.2f" % f)
#json.encoder.FLOAT_REPR = lambda f: format(f, ".6f")
#json.encoder.FLOAT_REPR = lambda f: format(Decimal(f), "f")

url_stg = '172.42.13.195:9988'
url_live = '172.41.11.60:9988'
# market_id = '2001011000000'
url = url_stg
pair = 'BTC-USD'
port = 30001

def gen_futures(pair, i) :
  c = {}
  c['book_name'] = 'Futures'
  c['port']      = port
  c['tick_sz']   = 0
  c['impliers']  = []
  suffix = i['marketCode'].split(pair)[1]
  expiry = suffix.split('-')[1]

  # leg1: Spread, leg2: Perp
  implier1 = {}
  implier1["bi_type"]       = "a_bid_implier"
  implier1["bi_priority"]   = 1
  implier1["bi_leg1"]       = pair + '-SPR-' + expiry + 'P-LIN'
  implier1["bi_leg2"]       = pair + '-SWAP-LIN'
  implier1["bi_maker_fees"] = 0
  implier1["bi_factor"]     = 0
  implier1["ai_type"]       = "a_ask_implier"
  implier1["ai_priority"]   = 1
  implier1["ai_leg1"]       = pair + '-SPR-' + expiry + 'P-LIN'
  implier1["ai_leg2"]       = pair + '-SWAP-LIN'
  implier1["ai_maker_fees"] = 0
  implier1["ai_factor"]     = 0

  c['impliers'].append(implier1)
  return c

def gen_perpetual() :
  c = {}
  c['book_name'] = 'Perpetual'
  c['port']      = port
  c['tick_sz']   = 0
  c['impliers']  = []

  # implier1 = {}
  # implier1["bi_type"]       = "b_bid_implier"
  # implier1["bi_priority"]   = 1
  # implier1["bi_leg1"]       = "Futures"
  # implier1["bi_leg2"]       = "Spread-Fut-Perp"
  # implier1["bi_maker_fees"] = 0
  # implier1["bi_factor"]     = 0
  # implier1["ai_type"]       = "b_ask_implier"
  # implier1["ai_priority"]   = 1
  # implier1["ai_leg1"]       = "Futures"
  # implier1["ai_leg2"]       = "Spread-Fut-Perp"
  # implier1["ai_maker_fees"] = 0
  # implier1["ai_factor"]     = 0

  # c['impliers'].append(implier1)
  return c

def gen_spread(pair, i) :
  c = {}
  c['book_name'] = 'Spread-Fut-Perp'
  c['port']      = port
  c['tick_sz']   = 0
  c['impliers']  = []
  suffix = i['marketCode'].split(pair + '-SPR')[1]
  expiry = suffix.split('-')[1][:-1]

  #leg1: Futures, leg2: Perp
  implier1 = {}
  implier1["bi_type"]       = "in_bid_implier"
  implier1["bi_priority"]   = 1
  implier1["bi_leg1"]       = pair + '-' + expiry + '-LIN'
  implier1["bi_leg2"]       = pair + '-SWAP-LIN'
  implier1["bi_maker_fees"] = 0
  implier1["bi_factor"]     = 0
  implier1["ai_type"]       = "in_ask_implier"
  implier1["ai_priority"]   = 1
  implier1["ai_leg1"]       = pair + '-' + expiry + '-LIN'
  implier1["ai_leg2"]       = pair + '-SWAP-LIN'
  implier1["ai_maker_fees"] = 0
  implier1["ai_factor"]     = 0

  c['impliers'].append(implier1)
  return c

def gen_spot(pair, i) :
  c = {}
  c['book_name'] = 'Spot'
  c['port']      = port
  c['tick_sz']   = 0
  c['impliers']  = []
  #suffix = i['marketCode'].split(pair + '-SPR')[1]

  #leg1: Perp, leg2: Repo
  implier1 = {}
  implier1["bi_type"]       = "repo_out_bid"
  implier1["bi_priority"]   = 1
  implier1["bi_leg1"]       = pair + '-SWAP-LIN'
  implier1["bi_leg2"]       = pair + '-REPO-LIN'
  implier1["bi_maker_fees"] = 0
  implier1["bi_factor"]     = 0
  implier1["ai_type"]       = "repo_out_ask"
  implier1["ai_priority"]   = 1
  implier1["ai_leg1"]       = pair + '-SWAP-LIN'
  implier1["ai_leg2"]       = pair + '-REPO-LIN'
  implier1["ai_maker_fees"] = 0
  implier1["ai_factor"]     = 0

  c['impliers'].append(implier1)
  return c

def gen_repo() :
  c = {}
  c['book_name'] = 'Repo'
  c['port']      = port
  c['tick_sz']   = 0
  c['impliers']  = []
  return c

def gen_index() :
  c = {}
  c['book_name'] = 'Index'
  c['port']      = port
  c['tick_sz']   = 0
  c['impliers']  = []
  return c


def check_expiry(inst):
  if 'listingDate' in inst and 'endDate' in inst :
    if inst['listingDate'] < time.time()*1000 <  inst['endDate']:
      return True
    else:
      return False
  return True

if __name__ == '__main__':
  print("=============================", flush=True)
  os.system('date +%Y-%m-%d_%H:%M:%S')
  url      = sys.argv[1]
  pair     = sys.argv[2]
  out_file = sys.argv[3]
  http_url = f'http://{url}/markets/pair/{pair}'

  res = requests.get(http_url)
  print(f'response: {res}', flush=True)

  if res.status_code == 200:
    print('OK!', flush=True)
  j = res.json()
  print(f'response: {j}', flush=True)

  c = {}
  c['underlying']  = pair
  c['node_id']     = -1
  c['instruments'] = []

  if len(j) > 0:
    for i in j :
      marketCode = i['marketCode']
      print(i)
      itype = i['type']
      if not check_expiry(i):
        print(f'Not between listingDate and endDate. Skipping...')
        continue

      if itype == 'FUTURE':
        if marketCode == pair + '-SWAP-LIN':
          perp = gen_perpetual()
          perp['tick_sz'] = Decimal(str(i['tickSize']))
          perp['market_id'] = i['marketId']
          perp['factor'] = i['factor']
          perp['maker_fees'] = i['makerFees']
          perp['market_code'] = marketCode
          c['instruments'].append(perp)
        else:
          fut = gen_futures(pair, i)
          fut['tick_sz'] = Decimal(str(i['tickSize']))
          fut['market_id'] = i['marketId']
          fut['factor'] = i['factor']
          fut['maker_fees'] = i['makerFees']
          fut['market_code'] = marketCode
          c['instruments'].append(fut)

      elif itype == 'REPO':
        repo = gen_repo()
        repo['tick_sz'] = Decimal(str(i['tickSize']))
        repo['market_id'] = i['marketId']
        repo['factor'] = i['factor']
        repo['maker_fees'] = i['makerFees']
        repo['market_code'] = marketCode
        c['instruments'].append(repo)

      elif itype == 'SPREAD':
        spr = gen_spread(pair, i)
        spr['tick_sz'] = Decimal(str(i['tickSize']))
        spr['market_id'] = i['marketId']
        spr['factor'] = i['factor']
        spr['maker_fees'] = i['makerFees']
        for im in spr['impliers']:
          im['bi_maker_fees'] = i['makerFees']
          im['ai_maker_fees'] = i['makerFees']
        spr['market_code'] = marketCode
        c['instruments'].append(spr)

      elif itype == 'SPOT':
        spot = gen_spot(pair, i)
        spot['tick_sz'] = Decimal(str(i['tickSize']))
        spot['market_id'] = i['marketId']
        spot['factor'] = i['factor']
        spot['maker_fees'] = i['makerFees']
        for im in spot['impliers']:
          im['ai_factor'] = i['factor']
          im['bi_factor'] = i['factor']
        spot['market_code'] = marketCode
        c['instruments'].append(spot)
        c['node_id'] = str(i['marketId'])[0:-12]

      elif itype == 'INDEX':
        spot = gen_index(pair, i)
        spot['tick_sz'] = Decimal(str(i['tickSize']))
        spot['market_id'] = i['marketId']
        spot['factor'] = i['factor']
        spot['maker_fees'] = i['makerFees']
        spot['market_code'] = marketCode
        c['instruments'].append(spot)
        c['node_id'] = str(i['marketId'])[0:-12]

      port += 1

  print(json.dumps(c, indent=2, use_decimal=True))
  with open(out_file, 'w') as outf:
    json.dump(c, outf, indent=2, use_decimal=True)
