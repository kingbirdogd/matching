import zlib

def check(bids, asks):
  bids_l = []
  bid_l = []
  count_bid = 1
  while count_bid <= 25:
    if count_bid > len(bids):
      break
    bids_l.append(bids[count_bid - 1])
    count_bid += 1
    # print(bid_l)
    # print(bids_l)
  for j in bids_l:
    str_bid = str(j[0]) + ":" + str(j[1])
    bid_l.append(str_bid)
    # print(bid_l)
  asks_l = []
  ask_l = []
  count_ask = 1
  while count_ask <= 25:
    if count_ask > len(asks):
      break
    asks_l.append(asks[count_ask - 1])
    count_ask += 1
  for k in asks_l:
    str_ask = str(k[0]) + ":" + str(k[1])
    ask_l.append(str_ask)
  num = ''
  if len(bid_l) == len(ask_l):
    for m in range(len(bid_l)):
      num += bid_l[m] + ':' + ask_l[m] + ':'
  elif len(bid_l) > len(ask_l):
    # bid档比ask档多
    for n in range(len(ask_l)):
      num += bid_l[n] + ':' + ask_l[n] + ':'
    for l in range(len(ask_l), len(bid_l)):
      num += bid_l[l] + ':'
  elif len(bid_l) < len(ask_l):
    # ask档比bid档多
    for n in range(len(bid_l)):
      num += bid_l[n] + ':' + ask_l[n] + ':'
    for l in range(len(bid_l), len(ask_l)):
      num += ask_l[l] + ':'
  new_num = num[:-1]
  print(new_num)
  int_checksum = zlib.crc32(new_num.encode())
  fina = change(int_checksum)
  return fina


def change(num_old):
  num = pow(2, 31) - 1
  if num_old > num:
    out = num_old - num * 2 - 2
  else:
    out = num_old
  return out

if __name__ == '__main__':
  # asks = [["3366.8", "9", 10],[ "3368","8",3]]
  # bids = [["3366.1", "7", 0], ["3366", "6", 3]]
  # asks = [[3366.8, 9, 10], [3368, 8, 3], [3372, 8, 3]]
  # bids = [[3366.1, 7, 0]]

  # asks = [[1300000000000, 1000000, 0, 0], [1400000000000, 2000000, 0, 0]]
  # bids = [[1226760000000, 11700000, 0, 0],[20000000, 900000, 0, 0]]
  #   {'instrumentId': 'BTC-USD', 'seqNum': 1597732472000152407, 'asks': [[13000, 0.01, 0, 0], [14000, 0.02, 0, 0]],
  #    'checksum': 617952496, 'bids': [[12267.6, 0.00266161, 0, 0], [0.2, 0.009, 0, 0]], 'timestamp': '1597742556496'}],
  #  'action': 'partial', 'table': 'futures/depth'}

  # asks = [[13000, 0.01, 0, 0], [14000, 0.02, 0, 0]]
  # bids = [[12267.6, 0.00266161, 0, 0], [0.2, 0.009, 0, 0]]
  #"checksum": -449555613, "instrument_id": 2001000000000, "seq_num": 1597732472000109757, "timestamp": "1597739730094"}], "table": "futures/depth"}
  #checksum string: 12267.6:0.117: 13000:0.01: 0.2:0.009: 14000:0.02

  # asks = []
  # bids = [[10284, 71.036, 0, 0]]

  asks = [[9887.69, 0, 0, 0],[9889.25, 30, 0, 1],[9890.01, 0, 0, 0]]
  bids = [[9888.3, 0, 0, 0],[9878.22, 0, 0, 0],[9878.06, 1, 0, 1]]

  crc32 = check(bids, asks)
  print(crc32)