# Technical Camera Documentation

This page provides an overview of the standards and libraries used to communicate with the camera, as well as a collection of technical information and manuals.

## Aravis

The plugin uses the latest stable release of Aravis (0.8.30) to interface with genicam cameras. The library provides a large support of genicam features. For more details please consult the [Aravis documentation](https://aravisproject.github.io/aravis/aravis-stable/) and the [aravis-detector developer docs](../../developer/index.md).

## The Gen\<I>Cam Standard

The Genicam standard provides modern cameras with a standard programming interface. While this standard applies to a wide range of devices, the AravisDetector Plugin is designed for GigE Vision cameras. More details are available at the [GenICam website](https://www.emva.org/standards-technology/genicam/).

## GigE Vision Camera installation

:::{important}
Remember to carefully read the provided installation guide when setting up a new device
:::

After connecting the camera to the ethernet port on your device go to Settings/Network and add a new Wired connection. Change the MTU value from automatic to 9000 (This is required for Jumbo packets, otherwise the overhead on each packet can start causing issues with data transfer) and the ipv4 method to link-local only.
