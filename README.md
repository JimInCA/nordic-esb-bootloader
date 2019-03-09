Nordic ESB w/Bootloader Firmware
===

This repository contains the code to build the firmware for the Nordic RF devices that support the Nordic proprietary Enhanced Shock Burst (ESB) protocol.  Included in this repository is a bootloader that is compatible with the ESB protocol.  This bootloader provides the ability to update the application firmware with an included loader.

So why ESB with the Nordic BLE device?  The answer is very simple; ESB has a higher bandwidth and lower over-head than BLE.

Repository Overview
---
There are a number of different parts to this repository that make up everything that you should need in order to build and test the esb firmware.  To aid in this objective, I've added a README to several of the sub-directories with instructions specific to that application.  

Here is a list of the different pieces of firmware and applications that make up  this package:

| Directory | Description |
| --- | --- |
| esb_transceiver  | This is the primary firmware for this repository.  What it is exactly is a uart to esb bridge for transferring data between two Nordic devices with both setup to support the ESB protocol. |
| esb_echo         | Echo program intended to be used for testing esb_transceiver.  What it does is echo back any valid esb packet that it receives. |
| esb_test         | Host application program for testing the  esb_transceiver firmware.  What it does is generates packets of data, sends the data using the uart, and then expects to receive that same packet back from the uart port.  Includes README with full instructions on how to build and run this test program. |
| bootloader       | Name says it all.  Includes README with full instructions on building the bootloader and application firmware. |
| hci_dfu_send_hex | Host program for loading a hex file onto the Nordic device that has the bootloader preloaded.  Includes a README with full instructions on building and using this application.  Please be aware that there is both a Python and C version of this program. |
| blinky           | Demo program that is intended for testing with the bootloader. |

The rest of this README will list the software that you'll need to install in order to build the firmware that will be loaded on the Nordic devices.

As stated above, this firmware can be built stand-alone or it can be built so that it supports a bootloader.  Please see the bootloader README for more information on building the firmware that will be used with the bootloader.

All firmware included here supports the Nordic PCA10031 dongle.  It's up to the user to port this firmware over to different Nordic devices of your choosing.

Installing the Build Environment
---

The best way to do this is to go to the Nordic website and follow their instructions on installing the required environment.  This will require you to go to http://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v14.x.x/.  When the page opens, click on nRF5_SDK_14.2.0_17b948a.zip to download and then expand the zip file.

Next, you'll need to install the ARM version of the gcc compiler.  You will first need to go to the ARM web site at https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads and download the correct version for your system.  Please be aware that there may be a problem with the latest version of the compiler.  The last version that I know of that will build the sources in this repository with out any errors is release 7.3.1.  You can try a later version, but be aware that you may get errors and that you may have to uninstall that version and go to an earlier version.

Installing Source Code
---
Once you have the build environment installed, both gcc and the Nordic SDK, you will need to install the sources from this GitHub repository.

The first thing that you will need to do is to create the following directory:
```
cd <dev_dir>/nRF5_SDK_14.2.0_17b948a/
mkdir development
cd development
```
Once you have that done, installing the sources is a simple process of going to this repository and selecting 'Clone or download', which instructs git to copy the path for into your clipboard.  Then go back to the terminal window where you created the directory above and enter the following git command (this assumes that you have git installed):
```
git clone https://github.com/JimInCA/nordic_esp.git
```
And that's it.  You should now have the sources installed on your system and they should be ready to build.

Updating Nordic source files
---

You'll first need to make a modification to one of the Nordic source files in order to get our sources to build.  You will need to open the following file and update as described:
```
<dev_dir>/nRF5_SDK_14.2.0_17b948a/components/proprietary_rf/esb/nrf_esb.h
```
Add the following #define:
```
#include "sdk_config.h"
```
Please be aware that you may also have to update the following file so that it points to the correct gcc version that you are using:
```
<dev_dir>/nRF5_SDK_14.2.0_17b948a/components/toolchain/gcc/Makefile.windows
```

Building the Source Code
---

Once you have everything installed and updated, you're ready for build our firmware.

To build the firmware, go to the following directory:
```
<dev_dir>/Nordic/nRF5_SDK_14.2.0_17b948a/development/nordic_esb/esb_transceiver/build/pca10031/armgcc
```
Building the source code is a simple matter of entering the following command at the prompt:
```
make all
```
The firmware binary will be placed in the '.\armgcc\_build' directory.

Installing the firmware
---

To install the firmware, all you will need to do is to have the dongle installed on your computer and enter the following command:
```
make flash
```
This should install the firmware on the dongle.  At least, I hope this will work.  I may have forgotten that the JLINK software needs to be installed, but I did this so long ago, that I can't remember.  If so, you can go to https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF5-Command-Line-Tools and install the proper tools.  

A Little Bit Of History
---
This code was originally developed to run under embedded Linux.  I had a lot of fun developing all the different facets of this project and wanted to release it so that others can use it as well.  But to make it more general, I ported it to Windows 10.  Although the documentation that I've provided shows how to build the code using arm-gcc for the Nordic code and gcc for the host programs, I've also included the proper uVision 5 project files so that you can build the Nordic code using Keil5 and Visual Studio solution files for the application code.  The choice as to which to use is up to you.

Have Fun!