Application Loader using Bootloader
===
hci_dfu_send_hex.py is a python program that can perform device firmware upgrade over HCI-UART interface. This application accepts firmware to be uploaded in hex format as a command line argument along with the COM Port to be used for the transport. 

Installing Python
---
The first thing that you'll need to do is to install the 2.7.x version of Python.  You should be able to do so by going to the Python download page at the following link and selecting the proper installer for your system.
```
https://www.python.org/downloads/release/python-2715/
```
Once you have Python installed, you will need to add two additional libraries.  One library adds support for the serial port and the other adds the libraries that support the Intel Hex format used by the hex files.  The simplest way to do this is with the Python pip installer that should have been added to your system when you installed Python.  The following three commands should do the trick.
```
python -m pip install --upgrade pip
pip install pyserial
pip install intelhex
```

Running the Loader
---
For this to work properly, the bootloader must be installed on your Nordic dongle and the bootloader must be enabled waiting for an image to be uploaded.  This may require taking the boot pin defined in the bootloader to its active state.

This program can take the following command line arguments:
```
--help, -h                       Show help message and exits.
--file FILE, -f FILE             Filename of Hex file. This is mandatory to provide.
--port COMPORT, -p COMPORT       COM Port to which the device is connected. This is mandatory to provide.
--flowcontrol, -fc               Enable flow control, default: disabled.
--baudrate BAUDRATE, -b BAUDRATE Desired baudrate 38400/96000/115200, default: 38400.
```
For and example, let's say that it is necessary to upload a firmware image 'blinky.hex' and the COMPORT on which the nRF device is connected is COM8, and flow control is to be used for transfer run the application with command line options.
```
$ python hci_dfu_send_hex.py -f ../../blinky/build/pca10031/armgcc/_build/blinky.hex -p COM8 -b 115200 -fc
Sending file ../../blinky/build/pca10031/armgcc/_build/blinky.hex to COM8, flow control = True
Progress:  0
Progress:  0
Progress:  2
Progress:  5
...
Progress:  91
Progress:  94
Progress:  100
```
Once the loader completes with 100% progress, the device's led should state flashing various colors.

Please note that HCI features like retransmission have not been implement in this program.

This program provides comprehensive messages to provide the progress of the firmware upgrade or the reason for failure in case the firmware did not succeed. However, this program is experimental.
