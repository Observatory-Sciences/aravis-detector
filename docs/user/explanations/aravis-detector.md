# Aravis-plugin

This section only covers instructions specific to the Aravis-Detector plugin. For a general overview of Odin Data frameProcessor plugins please refer to the [user](https://odin-detector.github.io/odin-data/master/user/explanations/frame-processor.html) and [developer](https://odin-detector.github.io/odin-data/master/developer/how-to/frame-processor-plugin.html) guides from the odin documentation.

## Plugin Setup

The Aravis-detector should be loaded first in the FrameProcessor plugin chain. It doesn't require the use of the frame receiver app as it directly handles buffers from the aravis camera. To load the plugin you can use the following json command:

```json
{   "plugin": {
    "load": {
        "index": "aravis",
        "name": "AravisDetectorPlugin",
        "library": "<Prefix Directory>/lib/libAravisDetectorPlugin.so"
}}}
```

:::{Note}
Here, index refers to the plugin alias for the frame processor and can be treated as a variable name. It will later be used to target the plugin when providing the fp with configuration values. The name field must be the same value as the main file name and library field should provide the FP with a complete path to the library file installed. Please consult the [build](../tutorials/build.md) documentation for installation information.
:::

The rest of the plugins can then be connected to the aravis-detector. Odin allows plugins to be connected in a tree structure, where plugins can be chained one to another or multiple plugins can be connected into one. For the default aravis plugin config file the live view and hdf plugins are both directly connected to it, similar to plugins 1 and 3 in the following example:

```json
{   "plugin": {
        "connect": {
            "index": "<other plugin 1>",
            "connection": "aravis"
}},
    "plugin": {
        "connect": {
            "index": "<other plugin 2>",
            "connection": "<other plugin 1>"
}},
    "plugin": {
        "connect": {
            "index": "<other plugin 3>",
            "connection": "<aravis>"
}}}
```

## Plugin Configurations

Different configurations can be passed at start up using the same config file from the setup process. The syntax used is:

```json
{ "aravis":{
        "list_devices": true,
        "compression": "none",
        "dataset": "data",
        "ip_address": "169.254.186.47"
}}
```

The following list is a complete set of available plugin configuration and their aliases:

| Config | Description| Default value |
|--------|------------|---------------|
| ip_address | Connects to the camera with the specified IP address | 127.0.0.1 |
| exposure_time | Sets the exposure time to the given value | 1000.0 us |
| frame_rate | Sets the frame rate to the given value | 5 Hz|
| frame_count | Sets a limit to the number of buffers acquired in continuos mode. 0 for no limit | 0|
| pixel_format | Sets the pixel format used by the camera | Mono8 |
| acquisition_mode | Sets the camera's acquisition mode. Not fully implemented | Continuous |
| aravis_callback | When set to true the camera sends signals when an mage buffers is finished. When set to false the plugin checks for buffers on a regular interval | True |
| data_set_name | name of the data set all frames are assigned to | No default|
| file_name | name of the file name all frames are assigned to | No default |
| compression | compression method used | No default |
| file_path | file path for all temporary files. Currently used by genicam | No default |
| start | start camera acquisition of buffers | value is ignored |
| stop | stop camera acquisition of buffers | value is ignored |
| list_devices | lists all genicam devices connected | value is ignored |
| frames | specifies a number of frames to acquire | deprecated, no default |

:::{note}
The recommended way to change plugin configuration is through the use of the odin-control server.
:::

## Supported genicam features

As of version 0.0.1, pre-release, the following features are implemented int the Aravis Plugin:

| Feature | Plugin support | API implementation | Notes|
|---------|----------------|--------------------|------|
| connecting to cameras| Can detect connected cameras and connect using the ip address | Only displays the model name of the camera| |
| Acquisition modes | Only Continuous mode is implemented at the moment | Uses continuous mode  only | The number of buffers acquired can be limited through the use of the variable frame_count (Number of frames on the web GUI). This value sounds similar to the frame_count function in Aravis which set the number of frames in MultiFrame mode. This is not implemented in the plugin and the number of frames is limited by stopping Continuous mode.|
| Frame rate | Can be set and read | Can be controlled through both the web GUI and arvcli| The plugin will not allow the user to set the frame rate above the hardware limit. In case the frame rate doesn't change please confirm the value is not beyond those bounds.|
| Exposure time | Can be set and read | Can be controlled through both the web GUI and arvcli| Similar to frame rate, the plugin will keep the exposure time within hardware bounds specified in the genicam xml file|
| Pixel format | Currently the software can read available formats and change them. | Not yet fully implemented |Some formats are not usable because they require further processing, like Mono12 |
| Resolution | Can be read | Is not displayed in the gui but it can be requested through the cli | Resolution can only be changed through AOI/ROI or Binning which are not yet implemented |
| XML readout | Automatically saves the XML file when connecting to camera | n/a |This feature is mostly useful for trouble shooting. Saves the xml file in the specified temporary folder  |

## Potential Features

The Aravis library already implements a large set of GenICam features. The following is a list of features that might get added in the future:

1. Binning/Decimation - groups pixels together to reduce the resolution of the image
2. ROI/AOI - scan the sensor in a limited region of interest of nxn pixels.
3. Trigger modes - change between software/ self/ external trigger.
4. Burst mode - saves images internally, allowing for a faster frame rate over a short period.
5. Offset - sensor signal cutoff value used to remove noise.
6. Read out control - changes the delay between frame capture and transfer.
