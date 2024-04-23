# Build

This software is a plugin extension to Odin data and requires that you install and build both Odin-data and Odin-control to be able to fully utilize this project. This page will provide instructions for installing these as well as the Aravis library.

:::{note}
This tutorial has been written for Ubuntu 22.4 Jellyfish. Compatibility with other Linux systems isn't guaranteed.
:::

## External Software Dependencies

The plugin has the following direct dependencies:

- [Aravis v0.8.31](https://github.com/AravisProject/aravis): used to interface with genicam cameras.
- [Cmake v2.9](https://cmake.org/): used to generate the make files.
- [Odin-data v1.10.1](https://github.com/odin-detector/odin-data): Aravis-detector inherits Odin's frameProcessor plugin structure and various utilities. Additionally, the frame processor app itself is required to run the plugin.
- [Odin-control v1.5.0](https://github.com/odin-detector/odin-control): Used as a control server. This project extends its functionality to include the aravis-detector plugin and provides a python CLI to communicate with the odin frameProcessor using the control server.

Secondary dependencies:

- Odin-data:
  - [Boost](https://www.boost.org/) used for multithreading.
  - [ZeroMQ](https://zeromq.org/) asynchronous message library.
  - [Log4CXX](https://logging.apache.org/log4cxx/latest_stable/)  message logs.
  - [HDF5](https://www.hdfgroup.org/HDF5) used in the file saver plugin.
- Aravis:
  - [Meson](https://mesonbuild.com/): build system
  - [GLib - 2.0](https://docs.gtk.org/glib/)
  - [GObject - 2.0](https://docs.gtk.org/gobject/)

## Installing sub-dependencies

To build all sub-dependencies you can run the following in your shell:

Odin Data:

```shell
sudo apt -y update
sudo apt install cmake python3.10-venv
sudo apt install libboost-program-options-dev libboost-filesystem-dev \
 libboost-date-time-dev libboost-dev libboost-system-dev \
 libboost-test-dev libboost-thread-dev libboost-regex-dev \
 libzmq3-dev libpcap-dev liblog4cxx-dev libblosc-dev \
 libhdf5-dev librdkafka-dev
```

Aravis:

```shell
sudo apt install ninja-build build-essential meson libxml2-dev libglib2.0-dev \
 libusb-1.0-0-dev gobject-introspection libgtk-3-dev \
 gtk-doc-tools xsltproc libgstreamer1.0-dev \
 libgstreamer-plugins-base1.0-dev \
 libgstreamer-plugins-good1.0-dev\
 libgirepository1.0-dev gettext
```

## Building Dependencies

(Optional) It's useful to save the plugin an it's dependencies in same folder and create a virtual environment for the python server. Create a shortcut to this folder under the name "cwd" as the rest of the guide utilizes it. Additionally you can create a temp folder for all the saved files. To do all this navigate to your desired location for the software and run the following:

```shell
mkdir odin_camera_driver && cd odin_camera_driver
cwd=$(pwd)
sudo apt-get update
mkdir temp
python3 -m venv venv
source /path/to/venv/bin/activate
```

For the next step make sure you have installed [git](https://git-scm.com/). It's important to use git clone for the odin-data library as you want to maintain the git history for versioning purposes. If you decide not to do this, you can download the tar version from git an unpack and build it, but after that you need to navigate into the build directory, include and manually edit the version.h file to a version of the form 1.1.1 , the exact numbers are not important. Do the same for Aravis-detector. Or simply run:

```shell
git clone https://github.com/AravisProject/aravis
git clone https://github.com/odin-detector/odin-control
git clone https://github.com/odin-detector/odin-data
git clone https://github.com/Observatory-Sciences/aravis-detector
```

Similar to "cwd" short cut, the commands written here use a prefix and it's shortcut:

```shell
 mkdir prefix
 PREFIX=$cwd/prefix
```

### Build Odin-data

```shell
PREFIX=$cwd/prefix
cd $cwd/odin-data
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=$PREFIX ../cpp
make -j4 && make install
ls -la $PREFIX
```

Don't forget to modify the -j4 flag in make to however many cores you want to use (or leave it out altogether).

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

This should run 7 tests with no errors and then you will activate the simulated camera provided by Aravis. To connect to the camera you can use the aravis app. If connected to a genicam you shou1ld be able to see it listed as well.

### Build aravis-detector

Run the following code to build the plugin library in the same directory as the rest of Odin:

```shell
 cd $cwd/aravis-detector
 mkdir build && cd build
 cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DODINDATA_ROOT_DIR=$PREFIX ../cpp
 make -j4 && make install
```

Install the aravis server extension:

```shell
 source venv/bin/activate
 cd $cwd/aravis-detector
 pip install -e python
```

Install the Python CLI:

```shell
 source venv/bin/activate
 cd $cwd/aravis-detector/python
 pip install -e tools
```
