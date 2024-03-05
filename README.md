# Aravis detector

An Odin detector for aravis compatible cameras.

---

- [Aravis detector](#aravis-detector)
  - [Install](#install)
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

1. Build aravis-detector

```shell
  mkdir build && cd build
  cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DODINDATA_ROOT_DIR=$PREFIX ../cpp
  make -j8 VERBOSE=1 && make install VERBOSE=1
  ls -la $PREFIX
```

## Dependencies

## Docs

## Contributions
