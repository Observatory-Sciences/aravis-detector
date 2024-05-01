# Main CLI implementation
from arvcli.control import get_HTTP_request, put_HTTP_request
from arvcli import __app_name__, __version__
from rich.progress import track
from ast import literal_eval
from typing import Optional
from rich import print
import yaml
import typer
import time
import os


APP_NAME = "arvcli"

app = typer.Typer()


def convert_type(s: str):
    """
    Converts a string to the closest type
    """
    try:
        return literal_eval(s)
    except ValueError:
        return s
    except SyntaxError:
        return s


def _version_callback(value: bool) -> None:
    """Returns current cli version"""

    # TODO: figure out how to get the odin version in here
    if value:
        typer.echo(f"{__app_name__} {__version__}")
        raise typer.Exit()


def _get_callback(path: str = None) -> None:
    """
    Callback function that wraps get_value

    Prints out the value returned by get_value and exits.

    Args:
        key: (silly_enum): a cmd_map key (maps cli command to http request)
    """
    if path is not None:
        result = get_value(path)
        if result is not None:
            name = path.split("/")
            print(f"[yellow]{name[-1]}[/yellow] = {result}")
        raise typer.Exit()


def _put_callback(path: str = None) -> None:
    """
    Callback function that wraps put_value

    Prints out the value returned by put_value and exits.

    Args:
        key: (silly_enum): a cmd_map key (maps cli command to http request)
    """
    if path is not None:
        val = input("Value = ")
        put_value(path, convert_type(val))
        status_time = int(get_value("fp/config/aravis/status_frequency_ms"))
        for value in track(
            range(round(status_time / 10)), description="Waiting for status..."
        ):
            time.sleep(0.01)
        result = get_value(path)
        if result is not None:
            name = path.split("/")
            print(f"[yellow]{name[-1]}[/yellow] = {result}")
        raise typer.Exit()


def get_value(path_temp: str) -> None:
    """
    Get a frame processor value using a path

    Separates the path into the http request path and the json request
    Sends a put request
    Parses the json response
    Returns the target value

    Args:
        path_temp (str): /fp/status/variable

    Returns:
        dictionary/single value
    """
    if path_temp is None:
        return
    path_split = path_temp.split("/")
    path = "/api/0.1/"
    json_path = []

    if path_split[0] == "fp":
        for item in path_split[0:3]:
            path += item + "/"
        json_path = ["value", 0]
        if len(path_split) >= 4:
            for item in path_split[3:]:
                json_path.append(item)

    elif path_split[0] == "aravis":
        for item in path_split:
            path += item + "/"
        if path_split[-1] == "config":
            json_path = ["config"]
        else:
            json_path = [path_split[-1], "value"]

    elif path_split[0] == "sys":
        for item in path_split:
            path += item + "/"
        if len(path_split) > 1:
            json_path = [path_split[-1]]

    # Send request and parse the data
    data = get_HTTP_request(path, configs["ip"], configs["port"])
    sub_data = data
    for item in json_path:
        sub_data = sub_data[item]
    return sub_data


def put_value(path_temp: str = None, value: str = None) -> None:
    """

    Workflow:

    if path is aravis/configure/<target> then does a direct call

    otherwise it separates it into a http path and a json tree
    then send the json tree as the http put request

    """
    path_split = path_temp.split("/")
    if path_split[0] == "aravis":
        response = put_HTTP_request(
            f"/api/0.1/{path_temp}", value, configs["ip"], configs["port"]
        )
    if path_split[0] == "fp":
        new_path = ""
        for item in path_split[0:-1]:
            new_path += item+'/'
        response = put_HTTP_request(
            f"/api/0.1/{new_path}", {path_split[-1]: value}, configs["ip"], configs["port"]
        )
    if "response" in response.keys():
        print(response["response"])
    if "error" in response.keys():
        print(response["error"])


def _status_callback(val: bool) -> None:
    """
    Returns the server status in json format and exits

    Args:
        val (bool): true when called
    """
    if val:
        _get_callback("fp/status")


def _config_callback(val: bool) -> None:
    """
    Returns the server config in json format and exits

    Args:
        val (bool): true when called
    """
    if val:
        _get_callback("fp/config")


def _ipaddress_callback(val: str) -> None:
    """
    Sets the ip address of the odin server

    Requires a command

    Args:
        val (str): ip address
    """
    if val is not None:
        configs["ip"] = val
        print("[yellow bold]Using server address[/yellow bold] is: ", val)


def _port_callback(val: str) -> None:
    """
    Sets the port of the odin server

    Requires a command

    Args:
        val (str): port
    """
    if val:
        configs["port"] = val
        print("[yellow bold]Using server port[/yellow bold] is: ", val)


@app.command()
def connect(
    camera_ip: Optional[str] = typer.Option(
        None, "-ip", "--ip_address", help="connect to camera with ip address"
    ),
    list: Optional[bool] = typer.Option(
        None, "-l", "--list", help="list all the cameras detected"
    ),
) -> None:
    """
    Connects the AravisDetector plugin to a camera.

    """
    if camera_ip is not None:
        response = put_HTTP_request(
            "/api/0.1/fp/config/aravis",
            {"ip_address": camera_ip},
            configs["ip"],
            configs["port"],
        )
        if "error" in response.keys():
            print(response["error"])
        status_time = int(get_value("fp/config/aravis/status_frequency_ms"))
        for value in track(
            range(round(status_time / 10) + 100), description="Waiting for status..."
        ):
            time.sleep(0.01)
        result = get_value("fp/config/aravis/camera_model")
        if result is not None:
            print(f"[yellow]Connected to camera: [/yellow] = {result}")
    if list:
        aravis_status = get_value("fp/status/aravis")
        for i in range(aravis_status["connected_devices"]):
            print(
                aravis_status[f"camera_{i}_id"],
                " = ",
                aravis_status[f"camera_{i}_address"],
            )


@app.command()
def stream(
    start: Optional[bool] = typer.Option(
        None, "-on", "--start", help="start acquiring frames in continuous mode"
    ),
    stop: Optional[bool] = typer.Option(
        None, "-off", "--stop", help="stop acquiring frames in continuous mode"
    ),
    n_frames: Optional[int] = typer.Option(
        None,
        "-c",
        "--capture",
        help="Acquire a fixed number of frames in continuous mode",
    ),
) -> None:
    """
    Control camera frame acquisition in continuous mode
    """
    if start:
        print("[green]Video starting[/green]")
        put_HTTP_request(
            "/api/0.1/aravis/config/start_acquisition",
            1,
            configs["ip"],
            configs["port"],
        )
    if stop:
        print("[red]Video stopping[/red]")
        put_HTTP_request(
            "/api/0.1/aravis/config/stop_acquisition", 1, configs["ip"], configs["port"]
        )
    if n_frames is not None:
        fps = get_value("aravis/config/frame_rate")
        time_per_frame = 1 / fps  # find how long it takes for a frame
        put_HTTP_request(
            "/api/0.1/aravis/config/frame_count",
            n_frames,
            configs["ip"],
            configs["port"],
        )
        put_HTTP_request(
            "/api/0.1/aravis/config/start_acquisition",
            1,
            configs["ip"],
            configs["port"],
        )
        for val in track(range(n_frames + 1), description="Acquiring..."):
            time.sleep(time_per_frame)
        frames_acquired = get_value("aravis/config/frame_captured")
        print(f"Acquired {frames_acquired} frames")


@app.command()
def hdf(
    start: Optional[bool] = typer.Option(None, "-s", "--start", help="start file writer"),
    stop: Optional[bool] = typer.Option(
        None, "-s", "--stop", help="stop the file writer"
    ),
    start_writing: Optional[bool] = typer.Option(
        None, "-w", "--write", help="start file writer and start acquisition"
    ),
    stop_writing: Optional[bool] = typer.Option(
        None, "-sw", "--stopW", help="stop file writer and acquisition"
    ),
    file_name: Optional[str] = typer.Option(
        None, "-f", "--file", help="saving file name"
    ),
    file_path: Optional[str] = typer.Option(
        None, "-p", "--path", help="path to the directory"
    ),
    num: Optional[int] = typer.Option(
        None, "-n", "--num", help="number of files to save"
    ),
) -> None:
    """
    Control the file writer plugin

    path and num default to config values

    name default to current time in run_seconds_minute_hour_day_month_year format
    """
    response = {}
    params = {
        "process": {"number": 1, "rank": 0},
        "master": "data",
        "acquisition_id": "",
        "file": {"extension": "h5"},
    }

    # exit quickly if stop is called
    if stop_writing:
        response = put_HTTP_request(
            "/api/0.1/fp/config/hdf/write", False, configs["ip"], configs["port"]
        )
        if response != {}:
            print(f"[/red] Error occurred: stop response: {response['error']}[/red]")
        else:
            print("[green]File writing has[/green] [red]stopped[/red]")
            put_HTTP_request(
                "/api/0.1/aravis/config/stop_acquisition",
                1,
                configs["ip"],
                configs["port"],
            )
            print("[red]Video stopping[/red]")

        return
    if stop:
        response = put_HTTP_request(
            "/api/0.1/fp/config/hdf/write", False, configs["ip"], configs["port"]
        )
        if response != {}:
            print(f"[/red] Error occurred: stop response: {response['error']}[/red]")
        else:
            print("[green]File writing was[/green] [red]stopped[/red]")
        return
    if num is None:
        num = get_value("fp/config/hdf/frames")
        if num == 0:
            num = configs["default_n_frames"]
            print(f"[yellow]Number of frames unspecified, using default {num}[/yellow]")
        else:
            print(f"[yellow]Number of frames unspecified, using previous value {num} [/yellow]")
    if file_name is None:
        cd = time.localtime()
        file_name = (
            f"run_{cd.tm_sec}s{cd.tm_min}m{cd.tm_hour}h"
            + f"{cd.tm_mday}d{cd.tm_mon}m{cd.tm_year}y"
        )
        print(
            f"[yellow]File name unspecified, using: [bold]{file_name}[/bold][/yellow]"
        )
    if file_path is None:
        file_path = get_value("fp/config/hdf/file/path")
        if file_path == "":
            file_path = configs["default_path"]
            print(
                f"[yellow]File path unspecified, using default: [bold]{file_path}[/bold][/yellow]"
            )
        else:
            print(
                f"[yellow]File path unspecified, using previous: [bold]{file_path}[/bold][/yellow]"
            )

    params["frames"] = num
    params["file"]["name"] = file_name
    params["file"]["path"] = file_path

    if start:
        params["write"] = True
        response = put_HTTP_request(
            "/api/0.1/fp/config/hdf", params, configs["ip"], configs["port"]
        )
        if response != {}:
            print("path response:", response["error"])
        else:
            print("[green]File writing has been [bold]armed[/bold][/green]")
        return
    if start_writing:
        params["write"] = True
        response = put_HTTP_request(
            "/api/0.1/fp/config/hdf", params, configs["ip"], configs["port"]
        )
        if response != {}:
            print("path response:", response["error"])
        else:
            print("[green]File writing has [bold]started[/bold][/green]")
            fps = get_value("aravis/config/frame_rate")
            time_per_frame = 1 / fps  # find how long it takes for a frame
            put_HTTP_request(
                "/api/0.1/aravis/config/frame_count",
                num,
                configs["ip"],
                configs["port"],
            )
            put_HTTP_request(
                "/api/0.1/aravis/config/start_acquisition",
                1,
                configs["ip"],
                configs["port"],
            )
            for val in track(range(num + 1), description="Acquiring..."):
                time.sleep(time_per_frame)
            frames_acquired = get_value("aravis/status/frames_captured")
            print(f"Acquired {frames_acquired} frames")

        return

    response = put_HTTP_request(
        "/api/0.1/fp/config/hdf", params, configs["ip"], configs["port"]
    )
    if response != {}:
        print("path response:", response["error"])
    else:
        print(
            f"[blue]The following parameters were set: [/blue] {num=}, {file_name=}, {file_path}"
        )


@app.command()
def http(
    put: Optional[str] = typer.Option(
        None, "-p", "--put", help="send a command to the server"
    ),
    value: Optional[str] = typer.Option(
        None, "-v", "--value", help="value to send to server"
    ),
    get: Optional[str] = typer.Option(
        None, "-g", "--get", help="request information from the server"
    ),
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

    """
    if put:
        if value is None:
            put_HTTP_request(put, 1, configs["ip"], configs["port"])
        else:
            put_HTTP_request(put, value, configs["ip"], configs["port"])
    if get:
        print(get_HTTP_request(get, configs["ip"], configs["port"]))


def initiate_config():
    """
    Read config file and create global variable
    """
    app_path = os.path.dirname(os.path.realpath(__file__))
    global config_path
    config_path = app_path + "/config.yaml"
    if not os.path.isfile(config_path):
        print("Config file doesn't exist yet")
    global configs
    with open(config_path, "r") as con_file:
        configs = yaml.safe_load(con_file)


@app.callback()
def main(
    status: Optional[bool] = typer.Option(
        None,
        "--status",
        "-s",
        case_sensitive=False,
        help="Print current status values of all plugins",
        callback=_status_callback,
    ),
    config: Optional[bool] = typer.Option(
        None,
        "--config",
        "-c",
        case_sensitive=False,
        help="Print current config values of all plugins",
        callback=_config_callback,
    ),
    ipaddress: Optional[str] = typer.Option(
        None,
        "--ip",
        "-i",
        case_sensitive=False,
        help="Specify an ip address for the server",
        callback=_ipaddress_callback,
        is_eager=True,
    ),
    port: Optional[str] = typer.Option(
        None,
        "--port",
        "-po",
        case_sensitive=False,
        help="Specify a port for the server",
        callback=_port_callback,
        is_eager=True,
    ),
    g: Optional[str] = typer.Option(
        None,
        "--get",
        "-g",
        case_sensitive=False,
        help=(
            "Print a specific value. Paths must be of the form: fp/{status/ config / aravis}/" +
            "<target> or aravis/{config/status}/<target>"
        ),
        callback=_get_callback,
    ),
    p: Optional[str] = typer.Option(
        None,
        "--put",
        "-p",
        case_sensitive=False,
        help=(
            "Change a specific value. Paths must be of the "
            + "form: fp/config/aravis/<target> or aravis/config/<target>"
        ),
        callback=_put_callback,
    ),
    version: Optional[bool] = typer.Option(
        None,
        "--version",
        "-v",
        help="Display arvcli's current version and exit",
        callback=_version_callback,
        is_eager=True,
    ),
) -> None:
    """
    Python CLI for Aravis-Detector Plugin

    For command detail call arvcli <command> --help
    """
    pass
