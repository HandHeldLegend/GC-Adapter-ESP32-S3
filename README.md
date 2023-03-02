# GC Pocket Adapter Firmware
This is the official Github page for the GC Pocket Adapter from Hand Held Legend.

## Software and PCB License
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/3.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/3.0/88x31.png" /></a><br />This firmware and PCB model are licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/3.0/">Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License</a>.

## 3D Models License

<a rel="license" href="http://creativecommons.org/licenses/by/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by/4.0/88x31.png" /></a><br />The 3D shell, buttons, and light pipe are licensed under a <a rel="license" href="http://creativecommons.org/licenses/by/4.0/">Creative Commons Attribution 4.0 International License</a>.

## Shop Links
- [GC Pocket Adapter Kit (Includes 3D Printed Parts)](https://handheldlegend.com/products/gc-pocket-adapter-adapter-for-gamecube-controllers)
- [PCB for GC Pocket Adapter (BYO 3D Parts)](https://handheldlegend.com/products/hardware-kit-for-gc-pocket-adapter)
- [Screw, Ball, and Spring Hardware Kit](https://handheldlegend.com/products/hardware-kit-for-gc-pocket-adapter)

## Detailed Information
All of our guides on assembly, updating the firmware, and operation
are listed on [our Wiki page linked here](https://wiki.handheldlegend.com/gc-pocket-adapter).

## 3D Parts
All 3D parts are in the [/3d directory](https://github.com/HandHeldLegend/GC-Adapter-ESP32-S3/tree/main/3d) on this Github repo in STEP format. A 3D STEP file is provided for accurate reference on the PCB dimensions.

For simpler printing with the smaller bits, we have array components for the buttons and light pipe in the [/3d/arrayed parts directory](https://github.com/HandHeldLegend/GC-Adapter-ESP32-S3/tree/main/3d/arrayed%20parts). These are prepared as STL files.

## Assembled Adapter

![Adapter Front](https://github.com/HandHeldLegend/GC-Adapter-ESP32-S3/blob/main/images/full_adapter.png?raw=true)
![Adapter Plug](https://github.com/HandHeldLegend/GC-Adapter-ESP32-S3/blob/main/images/gc_plug.png?raw=true)
![USB Plug](https://github.com/HandHeldLegend/GC-Adapter-ESP32-S3/blob/main/images/type_c.png?raw=true)

## Features
- 4 USB HID Input modes
- Web configurator to change settings
- Adapts GameCube controllers for use on Windows, Mac, Android, and Nintendo Switch
- Up to 1ms USB polling rate (Dependent on machine it's plugged in to)
- 1ms controller polling rate
- RGB LED indicator light
- Two buttons to add function to device

## Basic Operation
- When the device is plugged in, the LED light shows a color to indicate which mode it is in. 
	- Blue - DInput/Configuration Mode
	- Yellow - Nintendo Switch Mode
	- Purple - GameCube OEM Mode 
	- Green - XInputHID Mode
- With no controllers connected, pressing the left/right button will switch the current input mode.
- The light will fade to red to indicate it is in standby. Plug in a controller and the light will turn white to indicate the controller is detected.
