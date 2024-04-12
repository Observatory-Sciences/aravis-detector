import json
import requests

server_address = "http://127.0.0.1:"
port = "5025"


def put_HTTP_request(request: str, value: int):
    r = requests.put(server_address+port+request, json.dumps(value, separators=(',', ':')))
    return r.json()


def get_HTTP_request(request: str):
    r = requests.get(server_address+port+request)
    return r.json()
