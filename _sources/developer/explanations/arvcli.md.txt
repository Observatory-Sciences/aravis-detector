# Python CLI

Arvcli is a simple CLI interface that uses [typer](https://typer.tiangolo.com/) to interface with the command line. In essence, each functionality of the CLI is compartmentalized within a function. The general commands are divided in two sections:

1. Options: these commands are signalled by the use of a '-' flag and use the callback functions as implementation.
2. Commands: the commands each have their own subcommands and are created using the command decorator.

The app follows the usual folder structure of a python project:

- arvcli
  - \_\_init__.py
  - \_\_main__.py
  - cli.py
  - control.py
  - config.json
- setup.py

The init, setup and main files are only used to maintain the folder structure and don't serve a role in code logic. The control.py file implements two generic functions that use the request library to send HTTP put and get requests.

## cli.py

Most of the code is written in the cli.py file and loosely follows the general structure of a typer project.

### Main function

The main function has 6 different callback typer arguments:

- status (_status_callback): sends a get request for the entire frame processor status.
- config (_config_callback): sends a get request for the entire frame processor config.
- ipaddress (_ipaddress_callback): changes the default config.yml value for the server address
- port (_port_callback): changes the default config.yml value for the server port
- version (_version_callback): prints version and exits
- g (_get_callback): this function prints the output of the get_value function. It takes an api path then processes it into a valid http request. The json reply is then parsed to get the exact value
- p (_get_callback): this function sets a specific value.

### Get and Put requests

Any paths starting with the “fp” value are truncated after the 3rd value. The first part is added as part of the http request and the later parts are used to parse the json file.

In the above case, the “fp/config/aravis/” is transformed into “http:/< ip_address >/api/0.1/fp/config/aravis/” and the value frame_rate is used as a json key. “sys” paths are split immediately after sys, and aravis paths are sent as http paths directly.

All put requests are either of the form “aravis/config” or “fp/config/< plugin >/” with one more path key. The additional key is then used to create a json file sent to the server together with the input variable.

### Stream

The stream command is written within a function and has three possible subcommands:

- start: sends an http request to start a stream
- stop: sends an http request to stop the stream
- n_frames: set the frame acquired limit to a the specified number (required with the argument) and start acquiring. The function also calculates the time needed and displays a progress bar.

### hdf

The hdf command has several subcommands that allow the user to:

- start: prepares the file writer to start saving files when acquisition starts
- stop: stops the file writer plugin
- write: starts both file writing and acquisition
- stopW: stops write
- file: file name
- path: path of the file
- num: number of frames to save

The arguments are passed as normal values to the function and a set of if statements determine if they are used or not. The stop functions are parsed first, then path, num and name values. If this values are None the code sends a request to the server for the last value (except for num). If that value had not been set it defaults to the config defined values. The start options are checked last.

### http

The http function provides a the user with the option of sending http request to the server directly without using the cmap dictionary.
