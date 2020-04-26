blinky
===
The intent of the blinky application is for testing.  It will be used to test the build environment.  Later, it will be used for testing the initial installation of the bootloader.

Build Environment and Source Code
---
You will need to first install the build environment and the application source code as outlined in the README.md file for the root repository.  

Updating Nordic source files
---
You'll first need to make a modification to one of the Nordic source files in order for the blinky application to run properly.  You will need to open the following file and update as described:
```
<dev_dir>/nRF5_SDK_16.0.0_98a08e2/components/boards/pca10031.h
```
You will need to modify the following line from:
```
#define LEDS_ACTIVE_STATE 1
```
To:
```
#define LEDS_ACTIVE_STATE 0
```
For some unexplained reason, the configuration file for the pca10031 has this set to the wrong state.

Building the Source Code
---
Once you have everything installed and updated, you're ready to build the blinky firmware.

To build the firmware, go to the following directory:
```
<dev_dir>/nRF5_SDK_16.0.0_98a08e2/development/nordic-esb-bootloader/blinky/build/pca10031/armgcc
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
This should install the firmware on the dongle.  If the flashing of the firmware was successful, you should see the RGB LED start to flash RED, GREEN, and BLUE with an approximate 0.5 second delay between each flash.
