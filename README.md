Nordic ESB w/Bootloader Firmware
===

This repository contains the code to build the firmware for the Nordic RF devices that support the Nordic proprietary Enhanced Shock Burst (ESB) protocol.  Included in this repository is a bootloader that is compatible with the ESB protocol.  This bootloader provides the ability to update the application firmware with an included loader.

So why ESB with the Nordic BLE device?  The answer is very simple; ESB has a higher bandwidth and lower over-head than BLE.

Repository Overview
---
There are a number of different parts to this repository that make up everything that you should need in order to build and test the ESB firmware.  To aid in this objective, I've added a README to several of the sub-directories with instructions specific to that application.  

Here is a list of the different pieces of firmware and applications that make up  this package:

| Directory | Description |
| --- | --- |
| esb_transceiver  | A UART to ESB bridge for transferring data between two Nordic devices with both setup to support the ESB protocol. |
| esb_echo         | Echo program intended to be used for testing esb_transceiver. |
| esb_test         | Host application program for testing the esb_transceiver firmware. |
| bootloader       | Name says it all. |
| hci_dfu_send_hex | Host program for loading a hex file onto the Nordic device that has the bootloader preloaded. |
| blinky           | Demo program that is intended for testing with the bootloader. |

The rest of this README will list the software that you'll need to install in order to build the firmware that will be loaded on the Nordic devices.

As stated above, this firmware can be built stand-alone or it can be built so that it supports a bootloader.  Please see the bootloader README for more information on building the firmware that will be used with the bootloader.

All firmware included here supports the Nordic PCA10031 dongle.  It's up to the user to port this firmware over to different Nordic devices of your choosing.

Installing the Build Environment
---

The best way to do this is to go to the Nordic website and follow their instructions on installing the required environment.  This will require you to go to http://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v16.x.x/.  When the page opens, click on nRF5_SDK_16.0.0_98a08e2.zip to download and then expand the zip file.

Next, you'll need to install the ARM version of the gcc compiler.  You will first need to go to the ARM web site at https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads and download the correct version for your system.  Please be aware that there may be a problem with the latest version of the compiler.  The last version that I know of that will build the sources in this repository with out any errors is release 7.3.1.  You can try a later version, but be aware that you may get errors and that you may have to uninstall that version and go to an earlier version.

Installing Source Code
---
Once you have the build environment installed, both gcc and the Nordic SDK, you will need to install the sources from this GitHub repository.

The first thing that you will need to do is to create the following directory:
```
cd <dev_dir>/nRF5_SDK_16.0.0_98a08e2/
mkdir development
cd development
```
Once you have that done, installing the sources is a simple process of going to this repository and selecting 'Clone or download', which instructs git to copy the path for into your clipboard.  Then go back to the terminal window where you created the directory above and enter the following git command (this assumes that you have git installed):
```
git clone https://github.com/JimInCA/nordic_esp.git
```
And that's it.  You should now have the sources installed on your system and they should be ready to build.

About this Repository
---
The main focus of this repository is the bootloader.  But as listed above, there are a number of support programs that go along with this repo to help demonstrate and test the bootloader.  Here is a brief explanation of each of these programs:

#### blinky
 The simplest of the support programs is blinky.  This is just your standard 'Hello World' flash the LED type program.  The purpose of this program is more to test the installation and build tools than to provide a meaningful program.  But with that being said, if this builds, loads, and runs, then you're in good shape to proceed on to other more demanding programs.  
 
 This program can be build stand alone and runs on the pca10031 without the bootloader.  It includes a README.md file that documents how to build and flash the device with this program in stand alone mode.  Documentation on how to build this program to be used with the bootloader is included with the README.md in the bootloader directory.  I recommend that you test your setup with this program first prior to attempting anything else.

 #### esb_transceiver
 esb_transceiver is an application that uses the ESB protocol for wireless data transfer.  It transmits all data that it receives from its UART connection with the host to its ESB transmitter.  It also has the ability to receive data from the ESB receiver, which it then sends to its UART connection and on to the host.  Building and flashing esb_transceiver is documented in a separate README.md file in its directory. 

 As with the blinky application, esb_transceiver can be build stand alone and the README.md file documents this procedure.  If you want to build esb_transceiver to work alongside the bootloader, you will need to update the *.ld file for the application in the same manner as documented in the bootloader README.md file for blinky.

 #### esb_echo
 esb_echo is an application that echos back everything that it receives.  The intent of this application is that it will be used to test esb_transceiver.  Building and flashing esb_echo is documented in a separate README.md file in its directory. 

 #### esb_test 
 esb_test is a program that can be run on the host to test esb_transceiver.  What it does is it generates a packet of data, sends the packet of data to esb_transceiver by way of UART, and then expects to receive that same packet back through the UART.  It completes by writing out the results of the test with passing meaning that it received the packet as it was sent and failing indication that there was an error.

 Building and running esb_test is documented in a separate README.md file that's included in the directory for esb_test.

 #### bootloader
 This is the bootloader, which is the purpose for this repository.  This bootloader is designed so that it will work with the ESB application programs for the normal Nordic bootloader does not support ESB.  
 
 How to build and flash the bootloader to a pca10031 is documented in the README.md file in the directory for the bootloader.  The README.md also documents how to update the *.ld file for the blinky application.  But the same procedure needs to be performed for any application that needs to work alongside the bootloader.

#### hci_dfu_send_hex

This is the host program for loading a hex file onto the Nordic device that has the bootloader preloaded.  You will find a README.md in the directory for the hci_dfu_send_hex program with full instructions on building and using this application.  

The source directory for hci_dfu_send_hex includes both a c version and and python version of the program.  But programs works identically and the choice as to which one to use is upn the personal preferences of the user.

A Little Bit Of History
---
This code was originally developed to run under embedded Linux.  I had a lot of fun developing all the different facets of this project and wanted to release it so that others can use it as well.  But to make it more general, I ported it to Windows 10.  Although the documentation that I've provided shows how to build the code using arm-gcc for the Nordic code and gcc for the host programs, I've also included the proper uVision 5 project files so that you can build the Nordic code using Keil5 and Visual Studio solution files for the application code.  The choice as to which to use is up to you.

And most of all, Have Fun!