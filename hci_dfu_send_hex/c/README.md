Application Loader using Bootloader
===
This is a c based version of the hci_dfu_send_hex program that can be used for firmware updates over HCI-UART interface. This application accepts firmware to be in an Intel hex format as a command line argument along with the COM Port to be used for the transport.
This README documents the usage of the hci_dfu_send_hex program.  The intent of this program is to flash an application to the Nordic RF wireless devices.

Installing Required Software
---
You will need to download and install the gcc compiler along with the tools that you'll need to run make. You can do this through either Cygwin or MinGW.  The choice is up to you as to which one you want to use.  Here are some links to get you started:
```
https://sourceware.org/cygwin/
http://www.mingw.org/
```
To me, this always seems to be a little bit of a struggle for there isn't one single application installer that seems to do everything that you need.  So it gets to be a little bit of a hunt-and-peck process, but stick with it and you'll eventually get things to work properly.

Building the Test Program
---
Once you can run make and have it call gcc, building the program is as simple as entering the following command:
```
$ cd <path>/nordic_esb/hci_dfu_send_hex/c
$ make all
gcc -I./include -c -o obj/hci_dfu_send_hex.o src/hci_dfu_send_hex.c
Compiled obj/hci_dfu_send_hex.o successfully!
gcc -I./include -c -o obj/crc16pure.o src/crc16pure.c
Compiled obj/crc16pure.o successfully!
gcc -o  bin/hci_dfu_send_hex obj/hci_dfu_send_hex.o obj/crc16pure.o
Linking bin/hci_dfu_send_hex complete!
```

Loader Program Arguments
---
Here is an explanation of the different user arguments available for the loader application:
```
    -h      	Show help message and exits.
    -f FILE     Filename of Hex file. This is mandatory to provide.
    -p COMPORT  COM Port to which the device is connected. This is mandatory to provide.
    -f          Enable flow control, default: disabled.
    -b BAUDRATE Desired baudrate 38400/96000/115200, default: 38400.
```
Please be aware that the file and com port are mandatory.

Running the Loader Program
---
For this to work properly, the bootloader must be installed on your Nordic dongle and the bootloader must be enabled waiting for an image to be uploaded.  This may require taking the boot pin defined in the bootloader to its active state.

The blinky program will be used as an example.  To load the blinky application, you will need to enter the following command as shown below:
```
$ ./bin/hci_dfu_send_hex -f ../../blinky/build/pca10031/armgcc/_build/blinky.hex -p COM8 -b 115200
Successfully connected to UART on port COM8 at baud rate 115200.
DCB is ready for use.
Sending file ../../blinky/build/pca10031/armgcc/_build/blinky.hex to port COM8 at baudrate 115200
Progress:  31
Progress:  62
Progress:  93
Progress:  100
```
If all goes well, you should see 100% progress and the tri-color led on the dongle should start to flash different colors.

Please be aware that the load file for any application that's going to be loaded by the bootloader must be modified as called out in the README for the bootloader.
