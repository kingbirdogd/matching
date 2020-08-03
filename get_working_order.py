import requests
import sys

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f'Usage: {sys.argv[0]} RESTAPI_URL OUTPUT_FILE')
        sys.exit(-1)

    host     = sys.argv[1]
    out_file = sys.argv[2]
    #host = '172.42.13.195:9988'
    URL = f'http://{host}/workingorder/all'
    r = requests.get(url = URL)

    data = r.json()

    f = open(out_file, 'w')
    for order in data:
        print(order)
        f.write(str(order) + '\n')
    f.write("\n")
    f.close()
