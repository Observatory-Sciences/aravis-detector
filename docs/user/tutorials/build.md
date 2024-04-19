# Build

This software is a plugin extension to Odin data and requires that you install and build both Odin-data and Odin-control to be able to fully utilize this project. This page will provide instructions for installing these as well.

:::{note}
This tutorial has been written for Ubuntu 20.4 Jellyfish. Compatibility with other Linux system isn't guaranteed.
:::

## External Software Dependencies

The plugin has the following direct dependencies:

- Aravis
- Cmake
- Odin-data:
- Odin-control

Additionally, it inherits from Odin Data and Aravis the following:

- Odin-data:
  - Boost
  - ZeroMQ
  - Log4CXX
  - HDF5
  - Blosc/c-blosc
- Aravis:
  - Meson

## Installing sub-dependencies

To build all sub-dependencies you can run the following in your shell:

Odin Data:

```shell
sudo apt -y update
sudo apt install  cmake python3.10-venv
sudo apt install  libboost-program-options-dev libboost-filesystem-dev \
                  libboost-date-time-dev libboost-dev  libboost-system-dev \
                  libboost-test-dev libboost-thread-dev libboost-regex-dev \
                  libzmq3-dev libpcap-dev liblog4cxx-dev libblosc-dev \
                  libhdf5-dev librdkafka-dev
```

Aravis:

```shell
sudo apt install ninja-build build-essential meson libxml2-dev libglib2.0-dev \
                 libusb-1.0-0-dev gobject-introspection libgtk-3-dev \
                 gtk-doc-tools  xsltproc libgstreamer1.0-dev \
                 libgstreamer-plugins-base1.0-dev \
                 libgstreamer-plugins-good1.0-dev\
                 libgirepository1.0-dev gettext
```

## Building Dependencies

(Optional) Create a simple folder structure:

```shell
mkdir odin_camera_driver && cd odin_camera_driver
cwd=$(pwd)
sudo apt-get update
mkdir temp
python3 -m venv venv
source /path/to/venv/bin/activate
```

Navigate to your install folder and download the required libraries:

```shell
git clone https://github.com/AravisProject/aravis
git clone https://github.com/odin-detector/odin-control
git clone https://github.com/odin-detector/odin-data
git clone https://github.com/Observatory-Sciences/aravis-detector
```

While not necessary, the rest of these instructions use a prefix:

```shell
  mkdir prefix
  PREFIX=$cwd/prefix
  mkdir -p $PREFIX
```

### Build Odin-data

```shell
PREFIX=$cwd/prefix
cd $cwd/odin-data
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=$PREFIX ../cpp
make -j8 VERBOSE=1 && make install VERBOSE=1
ls -la $PREFIX
```

Note: don't forget to modify the -j4 flag in make to however many cores you want to use (or leave it out altogether)

### Install Odin Control

The use of a virtual python is recommended but not necessary:

```shell
source venv/bin/activate
pip install -e odin-control
odin_control
```

### Build Aravis

Install [Aravis](https://aravisproject.github.io/aravis/building.html) and switch to version 0.8.30:

```shell
cd your/workdir
git clone https://github.com/AravisProject/aravis
sudo apt install meson ninja-build
cd aravis
git checkout 96cea98
```

Build it using meson:

```shell
meson setup --prefix=$INSTALL_PREFIX --build.pkg-config-path $PC_ARAVIS build 
cd build
ninja
ninja install
```

To test your build:

```shell
meson test
cd src
arv-fake-gv-camera-0.8 -s GV02 -d all
```

This should run 7 tests with no errors and then you will activate the simulated camera provided by Aravis. To connect to the camera you can use the aravis app. If connected to a genicam you should be able to see it listed as well.

### Build aravis-detector

Run the following code to build the plugin library in the same directory with the rest of Odin:

```shell
  cd $cwd/aravis-detector
  mkdir build && cd build
  cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DODINDATA_ROOT_DIR=$PREFIX ../cpp
  make -j4 && make install
```

Install the aravis server:

```shell
  source  venv/bin/activate
  cd $cwd/aravis-detector
  pip install -e python
```
