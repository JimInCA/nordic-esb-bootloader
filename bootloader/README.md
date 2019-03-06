Building the bootloader
===

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
cd <dev_dir>/Nordic/nRF5_SDK_14.2.0_17b948a/development/multiplayer_rf/bootloader/build/pca10031/armgcc
make all
```
Loading the bootloader firmware onto the dongle is as simple as shown below:
```
make flash
```

And that's all folks...