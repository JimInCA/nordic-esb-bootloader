Bootloader
===
The purpose of a bootloader is to monitor the device when it first boots up to see if it needs to load a new application or run an application if one is already saved.

What this bootloader does is that it first initializes the device and then checks a designated boot pin.  If the boot pin is in the boot state, then it will wait to receive a new application from the UART connection.  If the boot pin is not in the boot state, it checks to see if there is a saved application.  If there is a saved application, it will transfer execution to that application.  If there is not a saved application, it will default to wait to receive a new application from the UART connection.  If the bootloader is waiting for an application and the host sends the device an application, the bootloader will save the application in flash.  Once the full application has been received, the bootloader will transfer execution to the application. 

In addition to this, which makes this bootloader somewhat unique is that it also needs to control the vector table after the application is started.  What is unique about this is that the bootloader needs to know if its in bootloader mode or if the application is executing to be able to determine which set of vectors need to be used; it's own or the applications.

Building and Using the Bootloader
===
What follows are the instructions on how to build the bootloader, how to build an application that supports the bootloader, how to flash a device with the bootloader, and loading an application using the bootloader.

To use the bootloader, it must be loaded into low memory.  The bootloader is designed to handle the vector table for both itself and for the application, hence the reason why it is loaded into low memory.  This as well means that the bootloader is limited in the amount of memory that it can use.  As a second point, this means that the application must be set up so that it will run in higher memory and not touch the lower memory that houses the bootloader.  When building using the arm gcc compiler, this means that the loader file must be set properly for both the bootloader and for the application.  Loader files typically end in *.ld as shown below for the bootloader:
```
<path>/nordic-esb-bootloader/bootloader/build/pca10031/armgcc/bootloader_gcc_nrf51.ld
```
Within the loader file, it is the MEMORY section that we need to set properly to instruct the loader where is the devices memory where we expect our program to be loaded.  The MEMORY structure for the bootloader is shown below:
```
MEMORY
{
  FLASH (rx) : ORIGIN = 0x0, LENGTH = 0x5000
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x2000
  bootloader_settings_page (r) : ORIGIN = 0x00005000, LENGTH = 0x0400
}
```
What we're telling the loader is that we want the bootloader to be configured to start at memory address 0x0 and that the size of the bootloader can not exceed 0x5000 addresses.  We're also instructing the loader that the bootloader will be using the first 0X2000 memory locations in ram and to reserve 0x400 bytes in flash for the bootloader_settings_page.  

The reason why I'm going into such detail is because we will be updating the loader file for the application and we need to know where in memory the bootloader is located so that we don't reallocate any of the bootloader's memory.  The bootloader won't like that and it will be a major pain in the back-side trying to debug. 

Updating Nordic source files
---
Building the bootloader requires the modification of a few more source files in the nordic SDK.  The bootloader requires the renaming of a few IRQ service routines.  The code is to be modified in such a way that it will build for either the bootloader or standard applications.

First, you'll need to modify the following file:
```
<dev_dir>/nRF5_SDK_16.0.0_98a08e2/modules/nrfx/soc/nrfx_irqs_nrf51.h
```
You'll first need to change the following lines from:
```
// UART0_IRQn
#define nrfx_uart_0_irq_handler     UART0_IRQHandler
```
To:
```
// UART0_IRQn
#ifdef BOOTLOADER_BUILD
#define nrfx_uart_0_irq_handler     UART0_IRQHandler_Bootloader
#else
#define nrfx_uart_0_irq_handler     UART0_IRQHandler
#endif
```
And next, you'll need to change the following lines from:
```
// GPIOTE_IRQn
#define nrfx_gpiote_irq_handler     GPIOTE_IRQHandler
```
To:
```
// GPIOTE_IRQn
#ifdef BOOTLOADER_BUILD
#define nrfx_gpiote_irq_handler     GPIOTE_IRQHandler_Bootloader
#else
#define nrfx_gpiote_irq_handler     GPIOTE_IRQHandler
#endif
```
Next, you'll need to modify this file:
```
<dev_dir>/nRF5_SDK_16.0.0_98a08e2/components/libraries/timer/app_timer.c
```
This file requires the following two changes from:
``` 
void RTC1_IRQHandler(void)

...

void SWI_IRQHandler(void)
```
To:
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

Building the bootloader
---
Once you have everything above competed, you should then be able to build the bootloader by going to the following directory and entering make all at the command prompt:
```
cd <dev_dir>/nRF5_SDK_16.0.0_98a08e2/development/nordic-esb-bootloader/bootloader/build/pca10031/armgcc
make all
```
Loading the bootloader firmware onto the dongle is as simple as shown below:
```
make flash
```

Building an Application that Supports a Bootloader.
---
The next step is knowing what you need to do to setup your application to run alongside the bootloader.  As indicated above, this will require the updating of the *.ld file to instruct the loader where we want the application to be loaded in system flash and what system ram is available for its use.  I included a blinky program in this repository and I will be using it to show what changes need to be make so that it will run with the bootloader.

The first step is to modify blinky's load file which is the following file:
```
<dev_dir>/nRF5_SDK_16.0.0_98a08e2/development/nordic-esb-bootloader/blinky/build/pca10031/armgcc/blinky_gcc_nrf51.ld
```
Here is what it should look like of no one modified this file without me knowing so:
```
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x40000
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x8000
}
```
The changes that we want to make are to both the flash and ram sections as shown below:
```
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00005400, LENGTH = 0x3AC00
  RAM (rwx) :  ORIGIN = 0x20002000, LENGTH = 0x6000
}
```
As you can see, we are telling the loader to start our application at flash address 0x5400 and that it can have a maximum length of 0x3AC00.  This is to account for the bootloader which is loaded in flash memory from address 0x0000 to 0x4FFF and uses address 0x5000 to 0x53FF for its settings page.  We also have to tell the bootloader that the available memory is reduced by this amount.  And the same thing is true for the ram in that we are instructing the loader that we want our application to use ram starting at address 0x20002000 and that there is a maximum of 0x6000 bytes for it to use.  And that's it for the changes to the application.  I don't think that you can get simpler than that.

The next step is to compile the application which can be done with a simple make all.  But just in case, you may want to do a make clean first to make sure that nothing is left over from an earlier build.  Yuo now should be ready to load your application using he bootloader.

Loading an Application using the Bootloader
---
Now we're finally getting to the point that we can load the application onto a dongle.  We actually have two choices here that we can use; make flash, or we can use hci_dfu_send_hex which is included in this repository.  The issue with make flash is that it doesn't use the bootloader to load the application.  So for this example, we will be using hci_dfu_send_hex to load the blinky application. 

There are actually two different versions of hci_dfu_send_hex for your use.  One is a python program and the other is a c program that needs to be compiled using Visual Studio.  I won't get into this anymore here for it should be documented in the README in the directory for the hci_dfu_send_hex programs.  Needless to say, both versions take the same command line arguments, so the choice is up to you.  

Let's start by going to the directory for the compiled version of hci_dfu_send_hex.

```
cd <path>/hci_dfu_send_hex/c/
```
The first thing that you'll need to do is to get the bootloader in an active state where is is waiting for an application to be loaded onto the device.  If you just flashed to bootloader as described above, this it should be in this state.  So assuming the directory structure defined in this repository, you should be able to load the blinky application with the following command and arguments:
```
$ ./x64/Debug/hci_dfu_send_hex -f ../../blinky/build/pca10031/armgcc/_build/blinky.hex -p COM8 -b 115200
Successfully connected to UART on port COM8 at baud rate 115200.
DCB is ready for use.
Sending file ../../blinky/build/pca10031/armgcc/_build/blinky.hex to port COM8 at baudrate 115200
Progress:  31
Progress:  62
Progress:  93
Progress:  100
```

If all goes well, hci_def_send_hex should complete with 100% progress and the dongle's led should start to flash red, green, and blue. 

That's all folks!