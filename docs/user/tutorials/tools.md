# Python Tools

As of version 0.0.1 prerelease the only available python tool is the Client Line Interface.

## ArvCLI

Aravis CLI allows the user to control the frame processor through the use of the command line interface. It also provides the user with an example of how to send http requests to the odin server. It can be installed into the virtualenv from the aravis detector directory. The environment should again be setup as described in the Run Tutorial:

```shell
export PATH=$MAIN_DIR/prefix/bin:$PATH
source $MAIN_DIR/venv/bin/activate
```

Then ```arvcli``` can be installed with: ```pip install python/tools -e```

The following options are available through the CLI:

```shell
$ arvcli --help
                                                                                               
 Usage: arvcli [OPTIONS] COMMAND [ARGS]...                                                     
                                                                                               
╭─ Options ───────────────────────────────────────────────────────────────────────────────────╮
│ --status              -s             Print current status values of all plugins             │
│ --config              -c             Print current config values of all plugins             │
│ --ip                  -i       TEXT  Specify an ip address for the server [default: None]   │
│ --port                -po      TEXT  Specify a port for the server [default: None]          │
│ --get                 -g       TEXT  Print a specific value. Paths must be of the form:     │
│                                      fp/{status/ config / aravis}/<target> or               │
│                                      aravis/{config/status}/<target>                        │
│                                      [default: None]                                        │
│ --put                 -p       TEXT  Change a specific value. Paths must be of the form:    │
│                                      fp/config/aravis/<target> or aravis/config/<target>    │
│                                      [default: None]                                        │
│ --version             -v             Display arvcli's current version and exit              │
│ --install-completion                 Install completion for the current shell.              │
│ --show-completion                    Show completion for the current shell, to copy it or   │
│                                      customize the installation.                            │
│ --help                               Show this message and exit.                            │
╰─────────────────────────────────────────────────────────────────────────────────────────────╯
╭─ Commands ──────────────────────────────────────────────────────────────────────────────────╮
│ connect   Connects the AravisDetector plugin to a camera.                                   │
│ hdf       Control the file writer plugin                                                    │
│ http      send custom HTTP requests                                                         │
│ stream    Control camera frame acquisition in continuous mode                               │
╰─────────────────────────────────────────────────────────────────────────────────────────────╯

```

### Requesting system information

The odin server pols the plugins for status and config values at a fixed frequency and stores the information locally. Arvcli can then directly request this information.

The commands ```--status``` and ```--config``` return the status and config values of all the plugins.

```shell
Status is: 
{
    'shared_memory': {'configured': False},
    'plugins': {'names': ['aravis', 'hdf', 'view']},
    'aravis': {
        'camera_id': '140519',
        'camera_connected': True,
        'payload': 20181312,
        'image_height': 3672,
        'image_width': 5496,
        'streaming': True,
        'input_buffers': 500,
        'output_buffers': 0,
        'frames_made': 439916,
        'completed_buff': 439907,
        'failed_buff': 0,
        'underrun_buff': 0,
        'timing': {'last_process': 0, 'max_process': 0, 'mean_process': 0}
    },
    'hdf': {
        'writing': False,
        'frames_max': 1000,
        'frames_written': 1000,
        'frames_processed': 1000,
        'file_path': 'odin_camera_driver/temp',
        'file_name': 'run_6s48m15h17d4m2024y_000000.h5',
        'acquisition_id': ' ',
        'processes': 1,
        'rank': 0,
        'timeout_active': False,
        'timing': {
            'last_create': 681,
            'max_create': 681,
            'mean_create': 530,
            'last_write': 26141,
            'max_write': 35680,
            'mean_write': 25260,
            'last_flush': 41,
            'max_flush': 66,
            'mean_flush': 40,
            'last_close': 64,
            'max_close': 246,
            'mean_close': 155,
            'last_process': 1,
            'max_process': 35801,
            'mean_process': 1
        }
    },
    'view': {'timing': {'last_process': 11, 'max_process': 43439, 'mean_process': 13}},
    'error': [
        'Frame invalid',
        'Frame destined for [data] but dataset has not been defined in the HDF plugin'
    ],
    'timestamp': '2024-04-18T16:30:11.005675',
    'connected': True
}
```

To get more specific values the user can call the ```--get``` command.

```shell
$arvcli --get fp/config/aravis/frame_rate
frame_rate = 5
```

To change more specific values the user can call the ```--put``` command.

```shell
$arvcli --put fp/config/aravis/frame_rate
Value = 10
Waiting for status... ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ 100% 0:00:01
frame_rate = 10
```

In both cases the path given is parsed by the python code and an http request and json path are formulated.

#### Get requests

Any paths starting with the "fp" value are truncated after the 3rd value. The first part is added as part of the http request and the later parts are used to parse the json file.

In the above case, the "fp/config/aravis/" is transformed into "http:/< ip_address >/api/0.1/fp/config/aravis/" and the value frame_rate is used as a json key. "sys" paths are split immediately after sys, and aravis paths are sent as http paths directly.

#### Put request

All put requests are either of the form "aravis/config" or "fp/config/< plugin >/" with one more path key. The additional key is then used to create a json file sent to the server together with the input variable.

#### Valid path list

The following is a list of example tree of paths that work with the aravis, live view and hdf plugin for the get function:

```shell
sys/
fp/ status  / aravis
            / view
            / hdf
    config  / aravis
            / view
            / hdf
aravis  / status
        / config
```

To each of this paths an additional value key can be added (eg. "frame_rate") to target a specific value. To get an accurate list of possible simply call the path as follows:

```shell
$arvcli --get fp/config/aravis
aravis = {'ip_address': '127.0.0.1', 'camera_id': '', 'camera_serial_number': '', 'camera_model': '',
        'exposure_time': 0.0, 'frame_rate': 0.0, 'frame_count': 1500, 'pixel_format': 'Mono8', 
        'acquisition_mode': 'Continuous', 'status_frequency_ms': 1000, 'empty_buffers': 50,
        'file_path': 'odin_camera_driver/temp', 'data_set_name': 'data', 'file_name': 'test'}
```

### Commands

Arvcli has subcommands (eg, `--get`, `--put`) that take one argument and commands. Commands are more complex and have their own subcommands.

To get a list of subcommands for a command simply use the `--help` tag after stating the command as follows:

```shell
arvcli connect --help
                                                                                        
 Usage: arvcli connect [OPTIONS]                                                        
                                                                                        
 Connects the AravisDetector plugin to a camera.                                                                    
                                                                                        
╭─ Options ────────────────────────────────────────────────────────────────────────────╮
│ --ip_address  -ip      TEXT  ip address of the GigE cam [default: None]              │
│ --list        -l             list all the cameras detected                           │
│ --help                       Show this message and exit.                             │
╰──────────────────────────────────────────────────────────────────────────────────────╯
```

### Frame acquisition

By "frame acquisition" we refer strictly to the capture of image buffers and their conversion to odin frame objects. These frames can then be saved by activating the hdf plugin (as explained in the following section).

The acquisition control is done through the ```stream``` command:

```shell
$ arvcli stream --help
                                                                                   
 Usage: arvcli stream [OPTIONS]                                                    
                                                                                   
 Control camera frame acquisition in continuous mode                               
                                                                                   
╭─ Options ───────────────────────────────────────────────────────────────────────╮
│ --start    -on                start acquiring frames in continuous mode         │
│ --stop     -off               stop acquiring frames in continuous mode          │
│ --capture  -c        INTEGER  Acquire a fixed number of frames in continuous    │
│                               mode                                              │
│                               [default: None]                                   │
│ --help                        Show this message and exit.                       │
╰─────────────────────────────────────────────────────────────────────────────────╯

```

### HDF plugin (saving images)

The HDF plugin can be controlled using the ```hdf``` command. The cli allows the user to either prepare the plugin for frame acquisition (arm) or prepare the plugin and start frame acquisition at the same time (start). Additionally, the path, name and number of saved frames can be specified using the following subcommands:

```shell
$ arvcli hdf --help
                                                                                   
 Usage: arvcli hdf [OPTIONS]                                                                                                                        
                                                                                   
╭─ Options ───────────────────────────────────────────────────────────────────────╮
│ --start  -s                start file writer                                    │
│ --stop   -s                stop the file writer                                 │
│ --write  -w                start file writer and start acquisition              │
│ --stopW  -sw               stop file writer and acquisition                     │
│ --file   -f       TEXT     saving file name [default: None]                     │
│ --path   -p       TEXT     path to the directory [default: None]                │
│ --num    -n       INTEGER  number of files to save [default: None]              │
│ --help                     Show this message and exit.                          │
╰─────────────────────────────────────────────────────────────────────────────────╯
```

The file name defaults to a string value of the form "run_<time stamp\>" and the path and file number default to the values specified in the config file.

### Settings and custom commands

Arvcli uses a config.yml file to store the IP address and port of the odin-control server as well as default values. This file can be edited directly (and this is encouraged) but the user can also change the IP address and port using the ```--ip``` and ```-port``` commands.

Additionally, the ```http``` command can be used to send direct put/get requests to the odin server:

```shell
$ arvcli http --help
                                                                                        
 Usage: arvcli http [OPTIONS]                                                           
                                                                                        
 send custom HTTP requests                                                              
 Use this for trouble shooting and custom functions                                     
 Api requests are of the following form:                                                
 /api/0.1/aravis/config/start_acquisition /api/0.1/aravis/config/stop_acquisition       
 /api/0.1/aravis/config/exposure_time /api/0.1/fp/config/hdf/write                      
 /api/0.1/fp/config/hdf/file/path /api/0.1/fp/config/hdf/file/name                      
 /api/0.1/fp/config/hdf/master /api/0.1/view/image 
 Args:
      put (str): an HTTP request  of the form: /api/0.1/aravis/config/start_acquisition     
      get (str): an HTTP request  of the form:                                                                           
                                                                                        
╭─ Options ────────────────────────────────────────────────────────────────────────────╮
│ --put    -p      TEXT  send a command to the server [default: None]                  │
│ --value  -v      TEXT  value to send to server [default: None]                       │
│ --get    -g      TEXT  request information from the server [default: None]           │
│ --help                 Show this message and exit.                                   │
╰──────────────────────────────────────────────────────────────────────────────────────╯

```
