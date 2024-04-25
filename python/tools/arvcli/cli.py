# Main CLI implementation
from arvcli.control import get_HTTP_request, put_HTTP_request
from arvcli import __app_name__, __version__
from rich.progress import track
from typing import Optional
from rich import print
from enum import Enum
import yaml
import typer
import time
import os

APP_NAME = "arvcli"

app = typer.Typer()

# Map the command to the http request and json path
cmd_map = {
    # It looks horrible, but it's quite simple.
    # Every entry has the following shape:
    # 'cli command name' : {'http': http api call, 'json': ['path', 'to file', 'in the json tree']}
    # The get_value function sends an api call and then, if successful, it gets back a json tree
    # that tree then gets parsed using the json path.

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
    'connected_devices': {'http': '/api/0.1/fp/status/aravis', 'json': ['value', 0,
                                                                        'connected_devices']},
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
"""
There is a very lovely feature in typer: predefined commands. This feature stops the user from
typing in wrong options to a command, and the lets them know of the correct ones.

There is one gigantic problem with it. It only uses enumerations. Which, for this particular case,
where you need to translate between a command and a specific value that cannot be a var name, it
completely falls apart!

My best attempt at fixing this was to create a decoy dictionary that just maps the keys together
and then convert that to a pointless enum.
It's all ridiculous and silly, hence the names:
"""
ridiculous_dict = {k: k for k in cmd_map.keys()}
silly_enum = Enum('silly_enum', ridiculous_dict)


def _version_callback(value: bool) -> None:
    """Returns current cli version"""

    # TODO: figure out how to get the odin version in here
    if value:
        typer.echo(f"{__app_name__} {__version__}")
        raise typer.Exit()


def _get_callback(key: silly_enum) -> None:
    """
    Callback function that wraps get_value

    Prints out the value returned by get_value and exits.

    Args:
        key: (silly_enum): a cmd_map key (maps cli command to http request)
    """

    if key is None:
        return
    val = get_value(key)
    print(f"[yellow bold]{key.value}[/yellow bold] is: ", val)
    raise typer.Exit()


def get_value(key: silly_enum) -> None:
    """
    Sends an api get request for the value specified then parses the json
    response and returns the target value

    Args:
        key: (silly_enum): a cmd_map key (maps cli command to http request)
    """
    data = get_HTTP_request(cmd_map[key.value]['http'], configs['ip'], configs['port'])
    sub_data = data
    for item in cmd_map[key.value]['json']:
        sub_data = sub_data[item]
    return sub_data


def _status_callback(val: bool) -> None:
    """
    Returns the server status in json format and exits

    Args:
        val (bool): true when called
    """
    if val:
        data = get_HTTP_request(cmd_map['status']['http'], configs['ip'], configs['port'])
        print("[yellow bold]Status[/yellow bold] is: ", data['value'][0])
        raise typer.Exit()


def _config_callback(val: bool) -> None:
    """
    Returns the server config in json format and exits

    Args:
        val (bool): true when called
    """
    if val:
        data = get_HTTP_request(cmd_map['config']['http'], configs['ip'], configs['port'])
        print("[yellow bold]Config[/yellow bold] is: ", data['value'][0])
        raise typer.Exit()


def _ipaddress_callback(val: str) -> None:
    """
    Sets the ip address of the odin server

    Requires a command

    Args:
        val (str): ip address
    """
    if val is not None:
        configs['ip'] = val
        print("[yellow bold]Using server address[/yellow bold] is: ", val)


def _port_callback(val: str) -> None:
    """
    Sets the port of the odin server

    Requires a command

    Args:
        val (str): port
    """
    if val:
        configs['port'] = val
        print("[yellow bold]Using server port[/yellow bold] is: ", val)


@app.command()
def connect(
    camera_ip: Optional[str] = typer.Option(None, "-ip", "--ip_address",
                                            help="using the ip address of the GigE cam"),
    list: Optional[bool] = typer.Option(None, "-l", "--list",
                                        help="list all the cameras detected")) -> None:
    """
    Not implemented. Connects the AravisDetector plugin to a camera.

    If no arguments are given it connects to the first available camera
    Currently I don't know if the server can specify which camera to use.
    Args:
        camera_ip (str): ip address . Defaults to typer.Option(None, "-ip", "--ip_address").
        camera_id (str): camera id. Defaults to typer.Option(None, "-id", "--name").
        camera_index (int): index. Defaults to typer.Option(None, "-ix", "--index").
    """
    if camera_ip is not None:
        print("[red]Unsuccessful, api not implemented yet[/red]")
        # print(f"Connecting to camera with address {camera_ip}.")
        return
    if list:
        aravis_status = get_value(silly_enum['aravis_status'])
        for i in range(aravis_status['connected_devices']):
            print(aravis_status[f'camera_{i}_id'], ' = ', aravis_status[f'camera_{i}_address'])


@app.command()
def stream(
    start: Optional[bool] = typer.Option(None, "-on", "--start",
                                         help="start acquiring frames in continuous mode"),
    stop: Optional[bool] = typer.Option(None, "-off", "--stop",
                                        help="stop acquiring frames in continuous mode"),
    n_frames: Optional[int] = typer.Option(None, "-t", "--take",
                                           help="Acquire a fixed number of frames in \
                                            continuous mode"),
                                       ) -> None:
    """
    Control camera frame acquisition in continuous mode
    """
    if start:
        print("[green]Video starting[/green]")
        put_HTTP_request("/api/0.1/aravis/config/start_acquisition", 1,
                         configs['ip'], configs['port'])
    if stop:
        print("[red]Video stopping[/red]")
        put_HTTP_request("/api/0.1/aravis/config/stop_acquisition", 1,
                         configs['ip'], configs['port'])
    if n_frames is not None:
        fps = get_value(silly_enum.frame_rate)
        time_per_frame = 1/fps  # find how long it takes for a frame
        put_HTTP_request("/api/0.1/aravis/config/frame_count", n_frames,
                         configs['ip'], configs['port'])
        put_HTTP_request("/api/0.1/aravis/config/start_acquisition", 1,
                         configs['ip'], configs['port'])
        for val in track(range(n_frames+1), description="Acquiring..."):
            time.sleep(time_per_frame)
        frames_acquired = get_value(silly_enum.frames_captured)
        print(f"Acquired {frames_acquired} frames")


@app.command()
def hdf(
    start: Optional[bool] = typer.Option(None, "-on", "--start",
                                         help="start saving frames"),
    arm: Optional[bool] = typer.Option(None, "-a", "--arm",
                                       help="start saving frames"),
    stop: Optional[bool] = typer.Option(None, "-off", "--stop",
                                        help="stop saving frames"),
    disarm: Optional[bool] = typer.Option(None, "-da", "--disarm",
                                          help="stop saving frames"),
    file_name: Optional[str] = typer.Option(None, "-f", "--file",
                                            help="saving file name"),
    file_path: Optional[str] = typer.Option(None, "-p", "--path",
                                            help="path to the directory"),
    num: Optional[int] = typer.Option(None, "-n", "--num",
                                      help="number of files to save")) -> None:
    """
    Control the file writer plugin

    Args:
        start (Optional[bool], optional): Starts acquiring and saving frames
        arm (Optional[bool], optional): Prepares the hdf plugin to save files as soon as
                                        acquisition starts.
        stop (Optional[bool], optional): Stops acquiring and saving files
        disarm (Optional[bool], optional): Stops saving files without stopping acquisition
        file_name (Optional[str], optional): sets the file name. Defaults to the current
                                             date and time as file name.
        file_path (Optional[str], optional): sets the file path. Will reuse the last value
                                             specified to the server or to the config value
        num (Optional[int], optional): sets the number of frames to save. Defaults to value
                                      given in configs.yaml
    """
    response = {}
    params = {
            'process': {
                'number': 1,
                'rank': 0
            },
            'master': 'data',
            'acquisition_id': '',
            'file': {
                'extension': 'h5'
            }
        }

    # exit quickly if stop is called
    if stop:
        response = put_HTTP_request('/api/0.1/fp/config/hdf/write', False,
                                    configs['ip'], configs['port'])
        if response != {}:
            print(f"[/red] Error occurred: stop response: {response['error']}[/red]")
        else:
            print("[green]File writing has[/green] [red]stopped[/red]")
            put_HTTP_request("/api/0.1/aravis/config/stop_acquisition", 1,
                             configs['ip'], configs['port'])
            print("[red]Video stopping[/red]")

        return
    if disarm:
        response = put_HTTP_request('/api/0.1/fp/config/hdf/write', False,
                                    configs['ip'], configs['port'])
        if response != {}:
            print(f"[/red] Error occurred: stop response: {response['error']}[/red]")
        else:
            print("[green]File writing was[/green] [red]disarmed[/red]")
        return
    if num is None:
        if num is None:
            num = configs['default_n_frames']
        print(f"[yellow]Number of frames unspecified, writing {num}[/yellow]")
    if file_name is None:
        cd = time.localtime()
        file_name = f'run_{cd.tm_sec}s{cd.tm_min}m{cd.tm_hour}h' + \
                    f'{cd.tm_mday}d{cd.tm_mon}m{cd.tm_year}y'
        print(f"[yellow]File name unspecified, using: [bold]{file_name}[/bold][/yellow]")
    if file_path is None:
        file_path = get_value(silly_enum.file_path)
        if file_path == '':
            file_path = configs['default_path']
        print(f"[yellow]File path unspecified, using: [bold]{file_path}[/bold][/yellow]")

    params['frames'] = num
    params['file']['name'] = file_name
    params['file']['path'] = file_path

    if arm:
        params['write'] = True
        response = put_HTTP_request('/api/0.1/fp/config/hdf', params,
                                    configs['ip'], configs['port'])
        if response != {}:
            print("path response:", response['error'])
        else:
            print("[green]File writing has been [bold]armed[/bold][/green]")
        return
    if start:
        params['write'] = True
        response = put_HTTP_request('/api/0.1/fp/config/hdf', params,
                                    configs['ip'], configs['port'])
        if response != {}:
            print("path response:", response['error'])
        else:
            print("[green]File writing has [bold]started[/bold][/green]")
            fps = get_value(silly_enum.frame_rate)
            time_per_frame = 1/fps  # find how long it takes for a frame
            put_HTTP_request("/api/0.1/aravis/config/frame_count", num,
                             configs['ip'], configs['port'])
            put_HTTP_request("/api/0.1/aravis/config/start_acquisition", 1,
                             configs['ip'], configs['port'])
            for val in track(range(num+1), description="Acquiring..."):
                time.sleep(time_per_frame)
            frames_acquired = get_value(silly_enum.frames_captured)
            print(f"Acquired {frames_acquired} frames")

        return

    response = put_HTTP_request('/api/0.1/fp/config/hdf', params, configs['ip'], configs['port'])
    if response != {}:
        print("path response:", response['error'])
    else:
        print(f"[blue]The following parameters were set: [/blue] {num=}, {file_name=}, {file_path}")


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
            put_HTTP_request(put, 1, configs['ip'], configs['port'])
        else:
            put_HTTP_request(put, value, configs['ip'], configs['port'])
    if get:
        get_HTTP_request(request=get)


def initiate_config():
    app_path = os.path.dirname(os.path.realpath(__file__))
    global config_path
    config_path = app_path+"/config.yaml"
    if not os.path.isfile(config_path):
        print("Config file doesn't exist yet")
    global configs
    with open(config_path, 'r') as con_file:
        configs = yaml.safe_load(con_file)


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
                                       help="Specify a port for the server",
                                       callback=_port_callback, is_eager=True),
    g: Optional[silly_enum] = typer.Option(None, '--get', '-g', case_sensitive=False,
                                           help="Print out a specific value",
                                           callback=_get_callback),
    version: Optional[bool] = typer.Option(None, "--version", "-v",
                                           help="Display arvcli's current version and exit",
                                           callback=_version_callback, is_eager=True)) -> None:

    pass
