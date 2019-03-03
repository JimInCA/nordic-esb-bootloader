Nordic ESB RF Firmware Repository
===

This repository contains the code to build the firmware for the Nordic RF device that uses the Nordic proprietary Enhanced Shock Burst (ESB) protocol.

Installing the Build Environment
---

The best way to do this is to go to the Nordic website and follow their instructions on installing the required environment.  This will require you to go to http://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v14.x.x/.  When the page opens, click on nRF5_SDK_14.2.0_17b948a.zip to download and then expand the zip file.

Next, you'll need to install the ARM version of the gcc compiler.  You will first need to go to the ARM web site at https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads and download the correct version for your system.  You have two different options here; you can do this under windows

Installing Source Code
---
Once you have the build environment installed, both gcc and the Nordic SDK, you will need to install the sources from this GitHub repository.

The first thing that you will need to do is to create the following directory:
```
cd <dev_dir>/nRF5_SDK_14.2.0_17b948a/
mkdir development
cd development
```
Once you have that done, installing our sources is a simple process of going to this repository and selecting 'Clone or download', which instructs git to copy the path for into your clipboard.  Then go back to the terminal window where you created the directory above and enter the following git command (this assumes that you have git installed):
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
<dev_dir>/nRF5_SDK_14.2.0_17b948a/components/toolchain/gcc/Makefile.posix
```

Building our Source Code
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
This should install the firmware on the dongle.  At least, I hope this will work.  I may have forgotten that the JLINK software needs to be installed, but I did this so long ago, that I can' remember.

Building the bootloader
---

Building the bootloader requires the modification of a few more source files in the nordic SDK.  The bootloader requires the renameing of a few IRQ service routines.  The code is to be modified in such a way that it will build for either the bootloader or standard applications.

First, you'll need to modify the following file:
```
<dev_dir>/Nordic/nRF5_SDK_14.2.0_17b948a/components/drivers_nrf/gpiote/nrf_drv_gpiote.c
```
You will need to change the following lines from:
```
void GPIOTE_IRQHandler(void)
{
```
To
```
#ifdef BOOTLOADER_BUILD
void GPIOTE_IRQHandler_Bootloader(void)
#else
void GPIOTE_IRQHandler(void)
#endif
{
```
The next change is to the following file:
```
<dev_dir>/Nordic/nRF5_SDK_14.2.0_17b948a/components/drivers_nrf/uart/nrf_drv_uart.c
```
You will need to to change the following line from:
``` 
#ifdef NRF52810_XXAA
    #define IRQ_HANDLER(n) void UARTE##n##_IRQHandler(void)
```

To
```
#if (defined(NRF52810_XXAA))
    #define IRQ_HANDLER(n) void UARTE##n##_IRQHandler(void))
#elif (defined(BOOTLOADER_BUILD))
    #define IRQ_HANDLER(n) void UART##n##_IRQHandler_Bootloader(void)
#else
    #define IRQ_HANDLER(n) void UART##n##_IRQHandler(void)
#endif
```

And then we will need to modify this file:
```
<dev_dir>/Nordic/nRF5_SDK_14.2.0_17b948a/components/libraries/timer/app_timer.c
```
This file requires the following two changes:
``` 
void RTC1_IRQHandler(void)

...

void SWI_IRQHandler(void)
```
``` 
#ifdef BOOTLOADER_BUILD
void RTC1_IRQHandler_Bootloader(void)
#else
void RTC1_IRQHandler(void)
#endif

...

#ifdef BOOTLOADER_BUILD
void SWI0_IRQHandler_Bootloader(void)
#else
void SWI_IRQHandler(void)
#endif
```

You should then be able to build the bootloader by going to the following directory and entering make all at the command prompt:
```
<dev_dir>/Nordic/nRF5_SDK_14.2.0_17b948a/development/multiplayer_rf/bootloader/build/pca10031/armgcc
```
