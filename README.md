# Aravis detector

![[Code CI](https://github.com/Observatory-Sciences/aravis-detector/actions/workflows/cpp.yml/badge.svg)](https://github.com/Observatory-Sciences/aravis-detector/actions/workflows/cpp.yml/badge.svg)
[![Apache License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

This project provides an [Odin Data](https://github.com/odin-detector/odin-data) plugin that controls [GenICam](https://www.genicam.org) devices, an [Odin Control](https://github.com/odin-detector/odin-control) server extension and a Python CLI tool. The plugin is ran within the the frame processor app in odin data and acquires buffers directly from the camera, circumventing the need for the frame receiver app. The Genicam interface is handled through the use of the [Aravis](https://github.com/AravisProject/aravis) library and each buffer captured from the camera passed through the normal Odin frame processor plugin chain making the detector compatible with other plugins designed to enhance the functionality of Odin Data.

---

- [Aravis detector](#aravis-detector)
  - [Install](#install)
  - [Dependencies](#dependencies)
  - [Docs](#docs)
  - [Contributions](#contributions)
  
---

## Install

## Dependencies

## Docs

## Contributions
