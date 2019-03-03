Application Loader using Bootloader
===
hci_dfu_send_hex.py is a python program that can perform device firmware upgrade over HCI-UART interface. This application accepts firmware to be upgraded in hex format as a command line argument along with the COM Port to be used for the transport. 

This program can take the following command line arguments:
```
--help, -h                       Show help message and exits.
--file FILE, -f FILE             Filename of Hex file. This is mandatory to provide.
--port COMPORT, -p COMPORT       COM Port to which the device is connected. This is mandatory to provide.
--flowcontrol, -fc               Enable flow control, default: disabled.
--baudrate BAUDRATE, -b BAUDRATE Desired baudrate 38400/96000/115200, default: 38400.
```
If it is required to upload a firmware image 'ble_app_hrs.hex' located at C:\NewFirmware, and the COMPORT on which the nRF device is connected is COM7, and flow control is to be used for transfer run the application with command line options.
```
python hci_dfu_send_hex.py -f  C:\NewFirmware\ble_app_hrs.hex -p COM7 -fc
```
Note 1: HCI features like retransmission have not been implement in this program.

This program provides comprehensive messages to provide the progress of the firmware upgrade or the reason for failure in case the firmware did not succeed. However, this program is experimental.

