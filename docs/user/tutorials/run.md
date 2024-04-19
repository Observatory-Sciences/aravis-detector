# Run

## Connect Aravis-detector plugin to Odin-data

Odin-data can be started from the terminal similar to any other app. Since the aravis plugin skips the frame receiver, only the frame processor is needed. To start it simply run:

```shell
$cwd/prefix/bin/frameProcessor --config $cwd/aravis-detector/examples/start_aravis.json
```

The ```--config``` flag specify a json configuration file that sets up the frame processor. More information on the possible configurations can be found in the [FrameProcessor documentation.](https://odin-detector.github.io/odin-data/master/user/explanations/frame-processor.html)

The default settings provided by the ```start_aravis.json``` file configure the plugin to list all available devices and then connect to the camera with the specified address. Make sure to connect any cameras before starting Odin, to do so you can follow the [guide given in explanations](../reference/camera). Additionally, the aravis camera simulator can be used to test the plugin by running:

```shell
$cwd/aravis/src/arv-fake-gv-camera-0.8 -s GV02 -d all
```

On a successful start the frame processor will output something similar to the following:

```shell
15:50:56,157  FP.App           INFO  - frameProcessor version 1.10.1 starting up
15:50:56,159  FP.SharedMemoryController TRACE - SharedMemoryController constructor.
15:50:56,194  FP.AravisDetectorPlugin INFO  - AravisDetectorPlugin loaded
15:50:56,194  FP.FrameProcessorController INFO  - Class AravisDetectorPlugin loaded as index = aravis
15:50:56,202  FP.LiveViewPlugin INFO  - LiveViewPlugin version 1.10.1 loaded
15:50:56,202  FP.LiveViewPlugin INFO  - Showing every 1 frame(s)
15:50:56,202  FP.LiveViewPlugin INFO  - Disabling Frames Per Second Option
15:50:56,202  FP.LiveViewPlugin INFO  - Setting the datasets allowed to: 
15:50:56,202  FP.LiveViewPlugin INFO  - Only Displaying images with the following tags: 
15:50:56,202  FP.FrameProcessorController INFO  - Class LiveViewPlugin loaded as index = view
15:50:56,254  FP.FileWriterPlugin INFO  - FileWriterPlugin version 1.10.1 loaded
15:50:56,254  FP.Acquisition   TRACE - Acquisition constructor.
15:50:56,254  FP.Acquisition   TRACE - Acquisition constructor.
15:50:56,254  FP.FrameProcessorController INFO  - Class FileWriterPlugin loaded as index = hdf
15:50:56,255  FP.LiveViewPlugin INFO  - Showing every 5 frame(s)
15:50:56,255  FP.LiveViewPlugin INFO  - Setting the datasets allowed to: data,
15:50:56,255  FP.LiveViewPlugin INFO  - Setting Live View Socket Address to tcp://0.0.0.0:5020
15:50:56,255  FP.LiveViewPlugin ERROR - Error binding socket to address tcp://0.0.0.0:5020 Error Number: 98
15:50:57,258  FP.AravisDetectorPlugin WARN  - No cameras were detected. Please confirm camera is connected
15:50:57,258  FP.FileWriterPlugin INFO  - {"params":{"dataset":{"data":{"datatype":"uint8","dims":[3672,5496],"compression":"none"}},"acquisition_id":"test","timeout_timer_period":3000},"msg_type":"cmd","msg_val":"configure","id":0,"timestamp":"2024-04-18T15:50:57.258893"}
15:50:57,258  FP.FileWriterPlugin INFO  - Checking for string name of dataset
15:50:57,258  FP.FileWriterPlugin INFO  - Dataset name data found, creating...
15:50:57,259  FP.FileWriterPlugin INFO  - Enabling compression: 1
15:50:57,259  FP.FileWriterPlugin INFO  - Setting next Acquisition ID to test
15:50:57,259  FP.FileWriterPlugin INFO  - Setting close file timeout to 3000
15:50:58,260  FP.AravisDetectorPlugin ERROR - No compatible cameras found, recheck connection
15:50:58,260  FP.FrameProcessorController INFO  - Running frame processor
```

## Start the control server

To start the Odin control server run the following in the same python environment you installed it in:

```shell
odin-control --config $cwd/aravis-detector/config/odin_server.cfg
```

By default the server gui will be accessible on http//localhost:8888/ amd the cli tool can be installed and used as described in [tools](tools).
