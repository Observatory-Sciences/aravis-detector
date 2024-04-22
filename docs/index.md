---
html_theme.sidebar_secondary.remove: True
---

# Aravis Detector Plugin

![[Code CI](https://github.com/Observatory-Sciences/aravis-detector/actions/workflows/cpp.yml/badge.svg)](https://github.com/Observatory-Sciences/aravis-detector/actions/workflows/cpp.yml/badge.svg)
[![Apache License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

This project provides an [Odin Data](https://github.com/odin-detector/odin-data) plugin that controls [GenICam](https://www.genicam.org) devices, an [Odin Control](https://github.com/odin-detector/odin-control) server extension and a Python CLI tool. The plugin is ran within the the frame processor app in odin data and acquires buffers directly from the camera, circumventing the need for the frame receiver app. The Genicam interface is handled through the use of the [Aravis](https://github.com/AravisProject/aravis) library and each buffer captured from the camera passed through the normal Odin frame processor plugin chain making the detector compatible with other plugins designed to enhance the functionality of Odin Data.

This project is aimed towards technical users. While it's not required, a general understanding of build system, odin-data and the genicam standard can help with any troubleshooting and should be sufficient for most use cases.

## How the documentation is structured

The documentation is split into 2 sections:

::::{grid} 2
:gutter: 4

:::{grid-item-card} {material-regular}`person;4em` User Guide
:link: ./user/index
:link-type: doc

Deploy and run the Aravis Detector plugin with Odin
:::

:::{grid-item-card} {material-regular}`code;4em` Developer Guide
:link: ./developer/index
:link-type: doc

Develop new camera features for the plugin
:::

::::
