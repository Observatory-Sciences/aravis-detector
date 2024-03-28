\page "User Guide"

\warning Currently this has only been tested on Ubuntu. It should in theory work on other systems but it will require tweaks and changes.

## Dev mode

This seems involved but it only involves using three terminals: one for the frameProcessor app, one for the fake camera (unless you have a real one), and one for the odin client to supply the config files during a run. Additionally you will need a json file to setup the processor and a config file for editing and changing it later. Also, make sure the paths are fine, this is more or less copied from my PC. You can do the following:

```shell
WORK_DIR = path/to/folder/containing/plugin
```

### Frame Processor

Compile the plugin:

```shell
cd /aravis-detector/build 
cmake -DCMAKE_INSTALL_PREFIX=${WORK_DIR}/prefix -DODINDATA_ROOT_DIR=${WORK_DIR}/prefix ../cpp
make -j8 && make install
```

\warning Make sure you replace fp.aravis.json with your own config file.

```shell
${WORK_DIR}/prefix/bin/frameProcessor \
  --ctrl tcp://0.0.0.0:5004 \
  --config ${WORK_DIR}/prefix/test_config/fp-aravis.json \
  --log-config ${WORK_DIR}/prefix/test_config/fp_log4cxx.xml 
```

\remark If you have the camera on before you start the processor you will get a connected message. Otherwise it's an error but that won't stop the app from running. After that you will need the client to connect to the camera.

### Fake Aravis Camera

Build Aravis, then turn on:

```shell
cd  ${WORK_DIR}/Aravis/aravis-0.8.30/build/src
arv-fake-gv-camera-0.8 -s GV02 -d all
```

### Odin-data client

For this step you need to change connect_cam.json with whatever file you hold your specifications in.

```shell
python3 ${WORK_DIR}/odin-data/python/src/odin_data/client.py \ 
        --ctrl tcp://0.0.0.0:5004  \
        --config ${WORK_DIR}/prefix/test_config/connect_cam.json
```