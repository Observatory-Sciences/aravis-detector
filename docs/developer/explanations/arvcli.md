# Python CLI

Arvcli is a simple CLI interface that uses [typer](https://typer.tiangolo.com/) to interface with the command line. In essence, each functionality of the CLI is compartmentalized within a function. The general commands are divided in two sections:

1. Direct callback: these commands are signalled by the use of a '-' flag and use the callback functions as implementation.
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
- g (_get_callback): this function prints the output of the get_value function. It uses the enum functionality of typer to automatically exclude wrong keywords and print out the correct set. The enumeration used is called silly_enum and maps the cmap keys with themselves.

### Get_value

This function allows an user to access a config/status variable by name. For this it uses the cmap dictionary to map the keyword argument with the right http request, then parses the json response and returns the most relevant part of it.

### Stream

The stream command is written within a function and has three possible subcommands:

- start: sends an http request to start a stream
- stop: sends an http request to stop the stream
- n_frames: set the frame acquired limit to a the specified number (required with the argument) and start acquiring. The function also calculates the time needed and displays a progress bar.

### hdf

The hdf command has several subcommands that allow the user to:

- set a file name, defaults to run_seconds_minutes_hours_day_month_year.
- set a file path, defaults to the value given in config.yml
- number of frames to save.

Additionally it can also start the hdf plugin with or without also starting frame acquisition.

### http

The http function provides a the user with the option of sending http request to the server directly without using the cmap dictionary.
