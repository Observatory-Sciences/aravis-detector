# Main CLI implementation
from arvcli.control import get_HTTP_request, put_HTTP_request
from arvcli import __app_name__, __version__
from rich.progress import track
from typing import Optional
from pathlib import Path
from rich import print
from enum import Enum
import typer
import time
import json

APP_NAME = "arvcli"
server_address = "192.168.1.194"
port = "5025"


app = typer.Typer()

# Map the command to the http request and json path
# The keys are the commands and they should be case insensitive
cmd_map = {
    # fp
    'fp': {'http': '/api/0.1/fp', 'json': []},
    # fp/config
    'config': {'http': '/api/0.1/fp/config', 'json': ['value']},
    # fp/config/aravis
    'camera_address': {'http': '/api/0.1/fp/config/aravis/ip_address', 'json': ['value', 0]},
    'frames': {'http': '/api/0.1/fp/config/hdf/frames', 'json': ['value']},
    'sys': {'http': '/api/0.1/sys', 'json': []},

    # fp/status
    'status': {'http': '/api/0.1/fp/status', 'json': ['value']},
    # fp/status/view
    'view_status': {'http': '/api/0.1/fp/status/view', 'json': ['value', 0]},
    'view_timing': {'http': '/api/0.1/fp/status/view', 'json': ['value', 0, 'timing']},
    # fp/status/hdf
    'hdf_status': {'http': '/api/0.1/fp/status/hdf', 'json': ['value', 0]},
    'writing': {'http': '/api/0.1/fp/status/hdf', 'json': ['value', 0, 'writing']},
    'frames_max': {'http': '/api/0.1/fp/status/hdf', 'json': ['value', 0, 'frames_max']},
    'frames_written': {'http': '/api/0.1/fp/status/hdf', 'json': ['value', 0, 'frames_written']},
    'frames_processed': {'http': '/api/0.1/fp/status/hdf',
                         'json': ['value', 0, 'frames_processed']},
    'file_path': {'http': '/api/0.1/fp/status/hdf', 'json': ['value', 0, 'file_path']},
    'file_name': {'http': '/api/0.1/fp/status/hdf', 'json': ['value', 0, 'file_name']},
    'acquisition_id': {'http': '/api/0.1/fp/status/hdf', 'json': ['value', 0, 'acquisition_id']},
    'processes': {'http': '/api/0.1/fp/status/hdf', 'json': ['value', 0, 'processes']},
    'rank': {'http': '/api/0.1/fp/status/hdf', 'json': ['value', 0, 'rank']},
    'timeout_active': {'http': '/api/0.1/fp/status/hdf', 'json': ['value', 0, 'timeout_active']},
    'hdf_timing': {'http': '/api/0.1/fp/status/hdf', 'json': ['value', 0, 'timing']},
    # fp/status/aravis
    'aravis_status': {'http': '/api/0.1/fp/status/aravis', 'json': ['value', 0]},
    'connected': {'http': '/api/0.1/fp/status/aravis', 'json': ['value', 0, 'camera_connected']},
    'img_height': {'http': '/api/0.1/fp/status/aravis', 'json': ['value', 0, 'image_height']},
    'img_width': {'http': '/api/0.1/fp/status/aravis', 'json': ['value', 0, 'image_width']},
    'input_buffers': {'http': '/api/0.1/fp/status/aravis', 'json': ['value', 0, 'input_buffers']},
    'output_buffers': {'http': '/api/0.1/fp/status/aravis', 'json': ['value', 0, 'output_buffers']},
    'completed_buffers': {'http': '/api/0.1/fp/status/aravis',
                          'json': ['value', 0, 'completed_buffers']},
    'failed_buffers': {'http': '/api/0.1/fp/status/aravis', 'json': ['value', 0, 'failed_buffers']},
    'underrun_buffers': {'http': '/api/0.1/fp/status/aravis',
                         'json': ['value', 0, 'underrun_buffers']},
    'aravis_timing': {'http': '/api/0.1/fp/status/aravis', 'json': ['value', 0, 'timing']},

    # http: aravis
    'aravis': {'http': '/api/0.1/aravis', 'json': []},
    # http: aravis/config
    'aravis_config': {'http': '/api/0.1/aravis/config', 'json': ['config']},
    'mode': {'http': '/api/0.1/aravis/config/mode', 'json': ['mode', 'value']},
    'frame_rate': {'http': '/api/0.1/aravis/config/frame_rate', 'json': ['frame_rate', 'value']},
    'frame_count': {'http': '/api/0.1/aravis/config/frame_count', 'json': ['frame_count', 'value']},
    'pixel_format': {'http': '/api/0.1/aravis/config/pixel_format',
                     'json': ['pixel_format', 'value']},
    'payload': {'http': '/api/0.1/aravis/status/payload_bytes', 'json': ['payload_bytes', 'value']},
    'exposure_time': {'http': '/api/0.1/aravis/config/exposure_time',
                      'json': ['exposure_time', 'value']},
    # http: aravis/status
    'camera_id': {'http': '/api/0.1/aravis/status/camera_id', 'json': ['camera_id', 'value']},
    'streaming': {'http': '/api/0.1/aravis/status/streaming', 'json': ['streaming', 'value']},
    'frames_captured': {'http': '/api/0.1/aravis/status/frames_captured',
                        'json': ['frames_captured', 'value']},
}

# There is a very lovely feature in typer: predefined commands. This feature stops the user from
# typing in wrong options to a command, and the lets them know of the correct ones.

# There is one gigantic problem with it. It only uses enumerations. Which, for this particular case,
# where you need to translate between a command and a specific value that cannot be a var name, it
# completely falls apart!

# My best attempt at fixing this was to create a decoy dictionary that just maps the keys together
# and then convert that to a pointless enum.
# It's all ridiculous and silly, hence the names:
ridiculous_dict = {k: k for k in cmd_map.keys()}
silly_enum = Enum('silly_enum', ridiculous_dict)


def _version_callback(value: bool) -> None:
    if value:
        typer.echo(f"{__app_name__} v{__version__}")
        raise typer.Exit()


def _get_callback(key: silly_enum) -> None:
    """
    Deals with the http request and json response

    Args:
        key: (silly_enum): value used to get
    """
    if key is None:
        return

    data = get_HTTP_request(cmd_map[key.value]['http'], server_address, port)
    sub_data = data
    for item in cmd_map[key.value]['json']:
        sub_data = sub_data[item]
    print(f"[yellow bold]{key.value}[/yellow bold] is: ", sub_data)
    raise typer.Exit()


def _status_callback(val: bool) -> None:
    if val:
        data = get_HTTP_request(cmd_map['status']['http'], server_address, port)
        print("[yellow bold]Status[/yellow bold] is: ", data['value'][0])
        raise typer.Exit()


def _config_callback(val: bool) -> None:
    if val:
        data = get_HTTP_request(cmd_map['config']['http'], server_address, port)
        print("[yellow bold]Config[/yellow bold] is: ", data['value'][0])
        raise typer.Exit()


def _ipaddress_callback(val: str) -> None:
    if val is not None:
        server_address = val
        print("[yellow bold]New server address[/yellow bold] is: ", val)
        raise typer.Exit()


def _port_callback(val: bool) -> None:
    if val:
        port = val
        print("[yellow bold]New server port[/yellow bold] is: ", val)
        raise typer.Exit()


# @app.command()
def connect(
    camera_ip: Optional[str] = typer.Option(None, "-ip", "--ip_address",
                                            help="using the ip address of the GigE cam"),
    camera_id: Optional[str] = typer.Option(None, "-id", "--name",
                                            help="using the manufacturers id"),
    camera_index: Optional[int] = typer.Option(None, "-ix", "--index",
                                               help="using the camera index")) -> None:
    """
    Not implemented. Connects the AravisDetector plugin to a camera.

    If no arguments are given it connects to the first available camera

    Args:
        camera_ip (str): ip address . Defaults to typer.Option(None, "-ip", "--ip_address").
        camera_id (str): camera id. Defaults to typer.Option(None, "-id", "--name").
        camera_index (int): index. Defaults to typer.Option(None, "-ix", "--index").
    """
    print("[red bold]Function not yet implemented[/red bold]")
    if camera_id is not None:
        print(f"Connecting to camera with id {camera_id}")
        return
    if camera_ip is not None:
        print(f"Connecting to camera with address {camera_ip}")
        return
    if camera_index is not None:
        print(f"Connecting to camera with index {camera_index}")
        return
    print("Connecting to the first camera available")


@app.command()
def stream(
    start: Optional[bool] = typer.Option(None, "-on", "--start",
                                         help="start acquiring frames in continuous mode"),
    stop: Optional[bool] = typer.Option(None, "-off", "--stop",
                                        help="stop acquiring frames in continuous mode"),
    n_frames: Optional[int] = typer.Option(None, "-t", "--take",
                                           help="Acquire a fixed number of frames in continuous mode"),
                                       ) -> None:
    """
    Control camera frame acquisition
    """
    if start:
        print("[green]Video starting[/green]")
        put_HTTP_request("/api/0.1/aravis/config/start_acquisition", 1, server_address, port)
    if stop:
        print("[red]Video stopping[/red]")
        put_HTTP_request("/api/0.1/aravis/config/stop_acquisition", 1, server_address, port)
    if n_frames is not None:
        fps = 5
        time_per_frame = 1/fps  # find how long it takes for a frame
        put_HTTP_request("/api/0.1/aravis/config/frame_count", n_frames, server_address, port)
        put_HTTP_request("/api/0.1/aravis/config/start_acquisition", 1, server_address, port)
        for val in track(range(n_frames+1), description="Acquiring..."):
            time.sleep(time_per_frame)
            if val % (fps*10) == 0 or val == n_frames:
                # every 10 seconds check progress

                print(f"Acquired {val} frames")


@app.command()
def hdf(
    start: Optional[bool] = typer.Option(None, "-on", "--start",
                                         help="start saving frames"),
    stop: Optional[bool] = typer.Option(None, "-off", "--stop",
                                        help="stop saving frames"),
    file_name: Optional[str] = typer.Option(None, "-f", "--file",
                                            help="saving file name"),
    file_path: Optional[str] = typer.Option(None, "-p", "--path",
                                            help="path to the directory"),
    files: Optional[int] = typer.Option(None, "-n", "--num",
                                        help="number of files to save"),) -> None:
    response = {}
    if stop:
        response = put_HTTP_request('/api/0.1/fp/config/hdf/write', 0, server_address, port)
        if response != {}:
            print("response:", response['error'])
    if files is not None:
        response = put_HTTP_request('/api/0.1/fp/config/hdf/frames', files, server_address, port)
        if response != {}:
            print("response:", response['error'])
    if file_name is not None:
        response = put_HTTP_request('/api/0.1/fp/config/hdf/name', file_name, server_address, port)
        if response != {}:
            print("response:", response['error'])
    if file_path is not None:
        response = put_HTTP_request('/api/0.1/fp/config/hdf/path', file_path, server_address, port)
        if response != {}:
            print("response:", response['error'])
    if start:
        response = put_HTTP_request('/api/0.1/fp/config/hdf/write', file_path, server_address, port)
        if response != {}:
            print("response:", response['error'])


@app.command()
def http(
    put: Optional[str] = typer.Option(None, '-p', '-put',
                                      help="send a command to the server"),
    value: Optional[str] = typer.Option(None, '-v', '-value',
                                        help="value to send to server"),
    get: Optional[str] = typer.Option(None, '-g', '-get',
                                      help="request information from the server")
) -> None:
    """
    send custom HTTP requests

    Use this for trouble shooting and custom functions

    Api requests are of the following form:

    /api/0.1/aravis/config/start_acquisition
    /api/0.1/aravis/config/stop_acquisition
    /api/0.1/aravis/config/exposure_time
    /api/0.1/fp/config/hdf/write
    /api/0.1/fp/config/hdf/file/path
    /api/0.1/fp/config/hdf/file/name
    /api/0.1/fp/config/hdf/master
    /api/0.1/view/image
    Args:
        put (str): an HTTP request of the form: /api/0.1/aravis/config/start_acquisition
        get (str): an HTTP request of the form:
    """
    if put:
        if value is None:
            put_HTTP_request(put, 1, server_address, port)
        else:
            put_HTTP_request(put, value, server_address, port)
    if get:
        get_HTTP_request(request=get)


@app.callback()
def main(
    status: Optional[bool] = typer.Option(None, '--status', '-s', case_sensitive=False,
                                          help="Print current status values of all plugins",
                                          callback=_status_callback),
    config: Optional[bool] = typer.Option(None, '--config', '-c', case_sensitive=False,
                                          help="Print current config values of all plugins",
                                          callback=_config_callback),
    ipaddress: Optional[str] = typer.Option(None, '--ip', '-i', case_sensitive=False,
                                            help="Specify an ip address for the server",
                                            callback=_ipaddress_callback, is_eager=True),
    port: Optional[str] = typer.Option(None, '--port', '-p', case_sensitive=False,
                                       help="SPecify a port for the server",
                                       callback=_port_callback, is_eager=True),
    g: Optional[silly_enum] = typer.Option(None, '--get', '-g', case_sensitive=False,
                                           help="Print out a specific value",
                                           callback=_get_callback),
    version: Optional[bool] = typer.Option(None, "--version", "-v",
                                           help="Display arvcli's current version and exit",
                                           callback=_version_callback, is_eager=True)) -> None:
    app_dir = typer.get_app_dir(APP_NAME)
    config_path: Path = Path(app_dir) / "config.json"
    global configs
    # configs = json.load(config_path)
    if not config_path.is_file():
        print("Config file doesn't exist yet")
