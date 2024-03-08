\mainpage Aravis Detector

Welcome to the documentation of Aravis detector!

## About project

This is a plugin designed for [Odin](github.com/odin-detector) that uses the [Aravis](aravisproject.github.io/aravis/introduction.html) library to access [GenICam](www.genicam.org) cameras.


## Dependencies

This project utilizes several different libraries to function, some of which are related to Odin and some to the Aravis library. Additionally, there are a few auxiliary dependencies such as those used to generate documentation and build files.

### Odin-data

The whole project is a plugin for the frameProcessor in [Odin-data](odin-detector.github.io/odin-data/master/index.html). Odin-data is a data acquisition framework that can provide modular support for large data streams. It has been previously used at Diamond for capturing and processing sensor data in projects like:

- [Excalibur](https://github.com/dls-controls/excalibur-detector)
- [Eiger](https://github.com/dls-controls/eiger-detector)
- [Tristan](https://github.com/dls-controls/tristan-detector)

Odin-data is composed of two communicating apps, the frameReceiver, a program that saves data to a buffer as quickly as possible to protect against packet loss, and the frameProcessor. When later app grabs the packets from the buffer, and sends down the buffer address down a pipeline of plugins that process the data on the buffer. Once the all the processing is done the data is written to disk and the buffer is freed for the frameReceiver.

This project will not use the frameReceiver (details TBA), and as such has a reduced list of dependencies:

 1. log4cxx
 2. zeromq
 3. boost


### Aravis

The Plugin uses the Aravis 0.8.3 library to control GenICam cameras. This Library has the following dependencies that need to be installed as well:

   1. Glib
   2. GObject

### Auxiliary

Build systems:

- CMake 2.9 for building Odin-data and AravisPlugin
- [Meson](https://mesonbuild.com/) for build Aravis

Documentation:

- [Doxygen](https://www.doxygen.nl/index.html) for C++ file documentation.
- [Doxygen Awesome](https://jothepro.github.io/doxygen-awesome-css/) for doxygen css and js files.
