import json
import requests


def put_HTTP_request(request: str, value: int, server_address: str, port: str):
    r = requests.put(f"http://{server_address}:{port}", json.dumps(value, separators=(',', ':')))
    return r.json()


def get_HTTP_request(request: str, server_address: str, port: str):
    r = requests.get(f"http://{server_address}:{port}")
    return r.json()
