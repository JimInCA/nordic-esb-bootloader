esb_transceiver
===
This is the main application that we will be using to test the bootloader.  But fist, we will be building esb_transceiver as a stand along application.  

Build Environment and Source Code
---
You will need to first install the build environment and the application source code as outlined in the README.md file for the root repository.  

Updating Nordic source files
---
You'll first need to make a modification to one of the Nordic source files in order to get our sources to build. You will need to open the following file and update as described:

<dev_dir>/nRF5_SDK_16.0.0_98a08e2/components/proprietary_rf/esb/nrf_esb.h

Add the following #define:

#include "sdk_config.h"

Please be aware that you may also have to update the following file so that it points to the correct gcc version that you are using:

<dev_dir>/nRF5_SDK_16.0.0_98a08e2/components/toolchain/gcc/Makefile.windows


Building the Source Code
---
Once you have everything installed and updated, you're ready to build the blinky firmware.

To build the firmware, go to the following directory:
```
<dev_dir>/nRF5_SDK_16.0.0_98a08e2/development/nordic-esb-bootloader/esb_transceiver/build/pca10031/armgcc
```
Building the source code is a simple matter of entering the following command at the prompt:
```
make all
```
The firmware binary will be placed in the './armgcc/_build' directory.

Installing the firmware
---

To install the firmware, all you will need to do is to have the dongle installed on your computer and enter the following command:
```
make flash
```
This should install the firmware on the dongle.  

Testing the Firmware
---
You will need to go to the esb_test directory and follow the README.md file in that directory for instructions on how to test the esb_transceiver and esb_echo firmware.
