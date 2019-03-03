hci_dfu_send_hex documentation
===

This README documents the usage of the hci_dfu_send_hex program.  The intent of this program is to flash an application to the Nordic RF wireless devices.

Here is an explanation of the different user parameters available with this flash tool:
```
    -h      	Show help message and exits.
    -f FILE     Filename of Hex file. This is mandatory to provide.
    -p COMPORT  COM Port to which the device is connected. This is mandatory to provide.
    -f          Enable flow control, default: disabled.
    -b BAUDRATE Desired baudrate 38400/96000/115200, default: 38400.
```

To flash a Nordic RF device, you will need to log on to LMVP and then enter the following command:
```
<path>/hci_dfu_send_hex -f <path>/lmvp_multiplayer_esb.hex -p COM8
```
