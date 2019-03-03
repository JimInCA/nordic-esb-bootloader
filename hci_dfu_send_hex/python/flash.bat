@echo off

SET HOME="C:\Users\Jim\Development"
SET PORT="COM4"
SET BAUDRATE="38400"

if "%1" == "blinky" (
	SET FILE="%HOME%\Nordic\nRF5_SDK_14.2.0_17b948a\development\nordic_esb\blinky\build\pca10031\keil5\_build\blinky.hex" 
) else if "%1" == "echo" (
	SET FILE="%HOME%\Nordic\nRF5_SDK_14.2.0_17b948a\development\nordic_esb\echo\build\pca10031\keil5\_build\echo.hex" 
) else if "%1" == "esb" (
	SET FILE="%HOME%\Nordic\nRF5_SDK_14.2.0_17b948a\development\nordic_esb\esb_transceiver\build\pca10031\keil5\_build\esb_transceiver.hex"
) else (
	echo Invalid input value.
	GOTO END
)

python %HOME%\Nordic\nRF5_SDK_14.2.0_17b948a\development\nordic_esb\hci_dfu_send_hex\hci_dfu_send_hex.py --file %FILE% --port %PORT% --baudrate %BAUDRATE%

:END
