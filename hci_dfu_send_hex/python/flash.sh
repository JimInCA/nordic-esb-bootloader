#!/bin/bash

PORT=COM8
BAUDRATE=115200
BUILDOS=keil5/_build

if [ "$1" == "blinky" ];
then
	FILE=$HOME/Development/Nordic/nRF5_SDK_14.2.0_17b948a/development/nordic_esb/blinky/build/pca10031/$BUILDOS/blinky.hex
elif [ "$1" == "echo" ];
then
	FILE=$HOME/Development/Nordic/nRF5_SDK_14.2.0_17b948a/development/nordic_esb/esb_echo/build/pca10031/$BUILDOS/echo.hex
elif [ "$1" == "esb" ];
then
	FILE=$HOME/Development/Nordic/nRF5_SDK_14.2.0_17b948a/development/nordic_esb/esb_transceiver/build/pca10031/$BUILDOS/esb_transceiver.hex
else
	echo Invalid input value.
	exit
fi

python $HOME/Development/Nordic/nRF5_SDK_14.2.0_17b948a/development/nordic_esb/hci_dfu_send_hex/python/hci_dfu_send_hex.py --file $FILE --port $PORT --baudrate $BAUDRATE

