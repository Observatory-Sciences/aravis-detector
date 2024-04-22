# AravisDetectorPlugin

The aravis plugin follows the general folder structure for a frameProcessor plugin:

- AravisPlugin
  - include
    - AravisDetectorPlugin.h
  - src
    - AravisDetectorPlugin.cpp
    - AravisDetectorPluginLib.cpp

## Flow diagram

Within the Odin-data stack frame processor plugins are normally called by the frame processor app whenever the frame receiver signals a new buffer. The Aravis Plugin bypasses this by acquiring the images directly from the camera. This process is enabled using a callback function that is automatically called by the GObject library function g_callback_function. The plugin then acquires the buffer, transforms it into a frame object and passes it along the chain (or tree) of plugins. A more general description can be seen in the following graph:

```{raw} html
:file: ../../images/AravisPlugin.svg
```

### Main process thread

The plugin is controlled by passing configuration values to the frame processor (see [user explanations](../../user/explanations/aravis-detector.md)). The config values are then parsed by the parse_configs function illustrated by the parse JSON configs block. As the config values are parsed, each value calls the corresponding function to execute the command. The graph does NOT illustrate each config and function and rather groups them together for clarity.

The start stream parameter enables continuous mode capture. To do this it allocates a group of buffer images in a stream object and enables callback signals from the camera. The camera then signals each finished buffer, which in turn start the buffer processing chain.

### Status thread

The status thread is started by the constructor and repeatedly polls the camera and stream objects for config values at a set frequency of 1 Hz.
