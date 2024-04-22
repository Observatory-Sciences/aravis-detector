# Python Tools

As of version 0.0.1 prerelease the only available python tool is the Client Line Interface.

## ArvCLI

Aravis CLI allows the user to control the frame processor through the use of the command line interface. It also provides the user with an example of how to send http requests to the odin server. It can be installed using ```pip install python/tools -e``` from the aravis detector directory.

The following options are available through the CLI:

```shell
$  arvcli --help

 Usage: arvcli [OPTIONS] COMMAND [ARGS]...                                      

╭─ Options ────────────────────────────────────────────────────────────────────╮
│ --status              -s                              Print current status   │
│                                                       values of all plugins  │
│ --config              -c                              Print current config   │
│                                                       values of all plugins  │
│ --ip                  -i      TEXT                    Specify an ip address  │
│                                                       for the server         │
│                                                       [default: None]        │
│ --port                -p      TEXT                    SPecify a port for the │
│                                                       server                 │
│                                                       [default: None]        │
│ --get                 -g      [fp|config|camera_addr  Print out a specific   │
│                               ess|frames|sys|status|  value                  │
│                               view_status|view_timin  [default: None]        │
│                               g|hdf_status|writing|f                         │
│                               rames_max|frames_writt                         │
│                               en|frames_processed|fi                         │
│                               le_path|file_name|acqu                         │
│                               isition_id|processes|r                         │
│                               ank|timeout_active|hdf                         │
│                               _timing|aravis_status|                         │
│                               connected|img_height|i                         │
│                               mg_width|input_buffers                         │
│                               |output_buffers|comple                         │
│                               ted_buffers|failed_buf                         │
│                               fers|underrun_buffers|                         │
│                               aravis_timing|aravis|a                         │
│                               ravis_config|mode|fram                         │
│                               e_rate|frame_count|pix                         │
│                               el_format|payload|expo                         │
│                               sure_time|camera_id|st                         │
│                               reaming|frames_capture                         │
│                               d]                                             │
│ --version             -v                              Display arvcli s       │
│                                                       current version and    │
│                                                       exit                   │
│ --install-completion                                  Install completion for │
│                                                       the current shell.     │
│ --show-completion                                     Show completion for    │
│                                                       the current shell, to  │
│                                                       copy it or customize   │
│                                                       the installation.      │
│ --help                                                Show this message and  │
│                                                       exit.                  │
╰──────────────────────────────────────────────────────────────────────────────╯
╭─ Commands ───────────────────────────────────────────────────────────────────╮
│ hdf      Control the file writer plugin                                      │
│ http     send custom HTTP requests                                           │
│ stream   Control camera frame acquisition in continuous mode                 │
╰──────────────────────────────────────────────────────────────────────────────╯
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
        'file_path': '/home/rfiodin/odin_camera_driver/temp',
        'file_name': 'run_6s48m15h17d4m2024y_000000.h5',
        'acquisition_id': '',
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

To get more specific values the user can call the ```--get``` command with one of the predefined keys:

```shell
$arvcli --get frames_captured
frames_captured is:  441306
```

The predefined keys correspond directly to values returned by the status and config commands, but arvcli uses specific http requests for each key and parses the json file in order to return the most specific response possible.

### Frame acquisition

By "frame acquisition" we refer strictly to the capture of image buffers and their conversion to odin frame objects. These frames can then be saved by activating the hdf plugin (as explained in the following section).

The acquisition control is done through the ```stream``` command:

```shell
$ arvcli stream --help
                                                                                
 Usage: arvcli stream [OPTIONS]                                                 
                                                                                
 Control camera frame acquisition in continuous mode                            
                                                                                
╭─ Options ────────────────────────────────────────────────────────────────────╮
│ --start  -on                start acquiring frames in continuous mode        │
│ --stop   -off               stop acquiring frames in continuous mode         │
│ --take   -t        INTEGER  Acquire a fixed number of frames in              │
│                             continuous mode                                  │
│                             [default: None]                                  │
│ --help                      Show this message and exit.                      │
╰──────────────────────────────────────────────────────────────────────────────╯

```

### HDF plugin (saving images)

The HDF plugin can be controlled using the ```hdf``` command. The cli allows the user to either prepare the plugin for frame acquisition (arm) or prepare the plugin and start frame acquisition at the same time (start). Additionally, the path, name and number of saved frames can be specified using the following subcommands:

```shell
$ arvcli hdf --help

 Usage: arvcli hdf [OPTIONS]                                                    
                                                                                
 Control the file writer plugin                                                 
 Args:     start (Optional[bool], optional): Starts acquiring and saving frames 
 arm (Optional[bool], optional): Prepares the hdf plugin to save files as soon  
 as                                     acquisition starts.     stop            
 (Optional[bool], optional): Stops acquiring and saving files     disarm        
 (Optional[bool], optional): Stops saving files without stopping acquisition    
 file_name (Optional[str], optional): sets the file name. Defaults to the       
 current                                          date and time as file name.   
 file_path (Optional[str], optional): sets the file path. Will reuse the last   
 value                                          specified to the server or to   
 the config value     num (Optional[int], optional): sets the number of frames  
 to save. Defaults to value                                   given in          
 configs.yaml                                                                   
                                                                                
╭─ Options ────────────────────────────────────────────────────────────────────╮
│ --start   -on                start saving frames                             │
│ --arm     -a                 start saving frames                             │
│ --stop    -off               stop saving frames                              │
│ --disarm  -da                stop saving frames                              │
│ --file    -f        TEXT     saving file name [default: run_]                │
│ --path    -p        TEXT     path to the directory [default: config]         │
│ --num     -n        INTEGER  number of files to save [default: None]         │
│ --help                       Show this message and exit.                     │
╰──────────────────────────────────────────────────────────────────────────────╯

```

The file name defaults to a string value of the form "run_<time stamp\>" and the path and file number default to the values specified in the config file.

### Settings and custom commands

Arvcli uses a config.yml file to store the IP address and port of the odin-control server as well as default values. This file can be edited directly (and this is encouraged) but the user can also change the IP address and port using the ```--ip``` and ```-port``` commands.

Additionally, the ```http``` command can be used to send direct put/get requests to the odin server:

```shell
 Usage: arvcli http [OPTIONS]                                                   
                                                                                
 send custom HTTP requests                                                      
 Use this for trouble shooting and custom functions                             
 Api requests are of the following form:                                        
 /api/0.1/aravis/config/start_acquisition                                       
 /api/0.1/aravis/config/stop_acquisition /api/0.1/aravis/config/exposure_time   
 /api/0.1/fp/config/hdf/write /api/0.1/fp/config/hdf/file/path                  
 /api/0.1/fp/config/hdf/file/name /api/0.1/fp/config/hdf/master                 
 /api/0.1/view/image Args:     put (str): an HTTP request of the form:          
 /api/0.1/aravis/config/start_acquisition     get (str): an HTTP request of the 
 form:                                                                          
                                                                                
╭─ Options ────────────────────────────────────────────────────────────────────╮
│         -p,-put        TEXT  send a command to the server [default: None]    │
│         -v,-value      TEXT  value to send to server [default: None]         │
│         -g,-get        TEXT  request information from the server             │
│                              [default: None]                                 │
│ --help                       Show this message and exit.                     │
╰──────────────────────────────────────────────────────────────────────────────╯
```
