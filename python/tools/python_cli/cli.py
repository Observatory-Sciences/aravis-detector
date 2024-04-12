# Main CLI implementation

from python_cli import __app_name__, __version__
from typing import Optional, Annotated, List
from enum import Enum
from rich import print
from python_cli.control import *
import typer 

app = typer.Typer()

glorious_dictionary = {
    'exposure_time': '/api/0.1/aravis/config/exposure_time'
}


########################
#  Callback functions  #
########################

def _version_callback(value: bool) -> None:
    if value:
        typer.echo(f"{__app_name__} v{__version__}")
        raise typer.Exit()

########################
#       Commnands      #
########################

@app.command()
def connect(
    camera_ip: Optional[str] = typer.Option(None, "-ip", "--ip_address",
                                            help="using the ip address of the GigE cam"),
    camera_id: Optional[str] = typer.Option(None, "-id", "--name",
                                            help="using the manufacturers id"),
    camera_index: Optional[int] = typer.Option(None, "-ix", "--index",
                                            help="using the camera index")
    ) -> None:
    """
    Not implemented. Connects the AravisDetector plugin to a camera.

    If no arguments are given it connects to the first available camera

    Args:
        camera_ip (Optional[str], optional): ip address . Defaults to typer.Option(None, "-ip", "--ip_address").
        camera_id (Optional[str], optional): camera id. Defaults to typer.Option(None, "-id", "--name").
        camera_index (Optional[int], optional): index. Defaults to typer.Option(None, "-ix", "--index").
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
                                        help="start acquiring frames in continous mode"),
    stop: Optional[bool] = typer.Option(None, "-off", "--stop",
                                        help="stop acquiring frames in continous mode"),
        
) ->None:
    """
    Control camera frame acquisition

    Args:
        start (Optional[bool], optional): _description_. Defaults to typer.Option(None, "-on", "--start", help="start acquiring frames in continous mode").
        stop (Optional[bool], optional): _description_. Defaults to typer.Option(None, "-off", "--stop", help="stop acquiring frames in continous mode").
    """
    if start:
        print("[green]Video starting[/green]")
        put_HTTP_request("/api/0.1/aravis/config/start_acquisition", 1)
    if stop:
        print("[red]Video stoping[/red]")
        put_HTTP_request("/api/0.1/aravis/config/stop_acquisition", 1)

@app.command()
def get(
    value: Optional[str] = typer.Option( "-v", "--value",
                                    help="returns the entire stream status")) -> None:
    """
    Returns useful information about the plugin or system

    Args:
        status (Optional[bool], optional): Defaults to typer.Option(None, "-s", "--status", help="returns the current stream status").
        config (Optional[bool], optional): Defaults to typer.Option(None, "-c", "--config", help="returns the current plugin config").
        devices (Optional[bool], optional): Defaults to typer.Option(None, "-d", "--devices", help="returns a list of all genicam devices connected").
    """
    data = get_HTTP_request(glorious_dictionary[value])
    print(data[value]["value"])

@app.command()
def http(
    put: Optional[str] = typer.Option(None, '-p', '-put',
                                help="send a comand to the server"),
    value: Optional[str] = typer.Option(None, '-v', '-value',
                                help="value to send to server"),
    get: Optional[str] = typer.Option(None, '-g', '-get',
                                help="request information from the server")
) -> None:
    """
    send costum HTTP requests

    Use this for trouble shooting and costum functions

    Api requests are of the following form:

    /api/0.1/aravis/config/start_aquistion
    /api/0.1/aravis/config/stop_aquistion
    /api/0.1/aravis/config/exposure_time
    /api/0.1/fp/config/hdf/write
    /api/0.1/fp/config/hdf/file/path
    /api/0.1/fp/config/hdf/file/name
    /api/0.1/fp/config/hdf/master
    /api/0.1/view/image
    Args:
        put (str): an HTTP request of the form: /api/0.1/aravis/config/start_aquistion
        get (str): an HTTP request of the form:
    """
    if put:
        if value is None:
            put_HTTP_request(put, 1)
        else:
            put_HTTP_request(put, value)
    if get:
        get_HTTP_request(request=get)

########################
#         Main         #
########################

@app.callback()
def main(
    version: Optional[bool] = typer.Option(
        None,
        "--version",
        "-v",
        help="Display arvcli's current version and exit",
        callback=_version_callback,
        is_eager=True)
)-> None:
    return
