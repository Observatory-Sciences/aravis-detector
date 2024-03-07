# Aravis detector

An Odin detector for aravis compatible cameras.

---

- [Aravis detector](#aravis-detector)
  - [Install](#install)
    - [Setup Aravis](#setup-aravis)
    - [Build aravis-detector](#build-aravis-detector)
  - [Dependencies](#dependencies)
  - [Docs](#docs)
  - [Contributions](#contributions)
  
---

## Install

**This install guide is temporary and only works for Ubuntu and is unlikely to work on different systems.**

1. Install dependencies:

```shell
  sudo apt -y update
  sudo apt install cmake
  sudo apt install libboost-program-options-dev libboost-filesystem-dev libboost-date-time-dev libboost-dev  libboost-system-dev libboost-test-dev libboost-thread-dev libboost-regex-dev
  sudo apt install libzmq3-dev libpcap-dev liblog4cxx-dev libblosc-dev libhdf5-dev librdkafka-dev
```

2. Make a prefix directory:

```shell
  PREFIX=path/to/folder/prefix
  mkdir -p $PREFIX
```

3. Build Odin-data:

```shell
  git clone https://github.com/odin-detector/odin-data.git && cd odin-data
  mkdir build && cd build
  cmake -DCMAKE_INSTALL_PREFIX=$PREFIX ../cpp
  make -j8 VERBOSE=1 && make install VERBOSE=1
  ls -la $PREFIX
  cd..
```

### Setup Aravis

In theory it should work with a normal aravis camera but I can't test that right now.

Here is how to quickly start the camera:

Install [Aravis](https://aravisproject.github.io/aravis/building.html):

```shell
cd your/workdir
git clone https://github.com/AravisProject/aravis
sudo apt install meson ninja-build
```

Change to version 0.8.30.
This is the last stable version of Aravis and the one I use. You can skip this step, but then
you need to change the name of the fake camera command to 0.10 and also change FindAravis.cmake
to look for 0.10. But I tested it and it works.

```shell
cd aravis
git checkout 96cea98
```

Build it:

```shell
meson setup build
cd build
ninja
ninja install
```

Run camera in a separate terminal:

```shell
cd your/workdir/build
cd src
arv-fake-gv-camera-0.8 -s GV02 -d all
```

### Build aravis-detector

```shell
  mkdir build && cd build
  cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DODINDATA_ROOT_DIR=$PREFIX ../cpp
  make -j8 VERBOSE=1 && make install VERBOSE=1
  ls -la $PREFIX
```

## Dependencies

1. Aravis
   1. Glib
   2. Gobject
2. odin-data
   1. log4cxx
   2. seromq

## Docs

## Contributions
