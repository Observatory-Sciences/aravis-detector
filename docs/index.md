---
html_theme.sidebar_secondary.remove: True
---

# Aravis Detector Plugin

![[Code CI](https://github.com/Observatory-Sciences/aravis-detector/actions/workflows/cpp.yml/badge.svg)](https://github.com/Observatory-Sciences/aravis-detector/actions/workflows/cpp.yml/badge.svg)
[![Apache License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

This project provides an [Odin Data](https://github.com/odin-detector/odin-data) plugin that controls [GenICam](https://www.genicam.org) devices, an [Odin Control](https://github.com/odin-detector/odin-control) server extension and a Python CLI tool. The plugin is ran within the the frame processor app in odin data and acquires buffers directly from the camera, circumventing the need for the frame receiver app. The Genicam interface is handled through the use of the [Aravis](https://github.com/AravisProject/aravis) library and each buffer captured from the camera is processed into a DataBlockFrame object and passed through the plugin chain. This allows compatibility with other Odin Data plugins, like the Live view and File writer plugins.

The project also provides the required python files to extend Odin Control, a tornado server api for Odin data. These files work as extensions to the odin server and provide the user with a web gui as well as support for http requests. The aravis command line interface (arvcli) uses this functionality to request config and status values from the Aravis plugin and to send control messages. More information on each of these is provided in the user guide.

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
