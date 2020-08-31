import simplejson as json
import os
import sys

#RUN echo "{\"ROOT_DIR\":\"/app\", \"REST_URL\":\"172.42.13.195:9988\", \"PERP_ID\":\"2001011000000\", \"REPO_ID\":\"2001031000000\"}" > /app/run_auction.json
if __name__ == '__main__':
  print("=============================", flush=True)
  os.system('date +%Y-%m-%d_%H:%M:%S')

  jcfg     = open(sys.argv[1])
  out_file = sys.argv[2]

  c = {}
  c['ROOT_DIR'] = os.environ['APPDIR']
  c['REST_URL'] = os.environ['REST_URL']

  j = json.load(jcfg)
  for i in j['instruments']:
    if i["book_name"] == "Perpetual":
      c['PERP_ID'] = i['market_id']
    elif i["book_name"] == "Repo":
      c['REPO_ID'] = i['market_id']

  print(json.dumps(c, indent=2, use_decimal=True))
  with open(out_file, 'w') as outf:
    json.dump(c, outf, indent=2, use_decimal=True)
