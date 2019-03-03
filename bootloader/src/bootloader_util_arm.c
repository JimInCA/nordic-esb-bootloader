/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include <stdint.h>

#include "bootloader_util.h"
#include "nordic_common.h"
#include "bootloader_types.h"
#include "dfu_types.h"


#if defined (__CC_ARM )
    uint8_t m_boot_settings[CODE_PAGE_SIZE] __attribute__((at(BOOTLOADER_SETTINGS_ADDRESS))) __attribute__((used));
#elif defined ( __GNUC__ )
    uint8_t m_boot_settings[CODE_PAGE_SIZE] __attribute__((section(".bootloader_settings_page"))) __attribute__((used));
#else
    #error Not a valid compiler/linker for m_dfu_settings placement.
#endif // Compiler specific


#if defined ( __CC_ARM )
__asm void StartApplication(uint32_t start_addr)
{
    LDR   R2, [R0]               ; Get App MSP.
    MSR   MSP, R2                ; Set the main stack pointer to the applications MSP.
    LDR   R3, [R0, #0x00000004]  ; Get application reset vector address.
    BX    R3                     ; No return - stack code is now activated only through SVC and plain interrupts.
    ALIGN
}
#elif defined ( __GNUC__ )
void __attribute__ (( naked )) StartApplication(uint32_t start_addr)
{
    __asm volatile(
        "ldr    r2, [r0]\t\n"         // Get App MSP.
        "msr    msp, r2\t\n"          // Set the main stack pointer to the application MSP.
        "ldr    r3, [r0, #0x04]\t\n"  // Get application reset vector address.
        "bx     r3\t\n"               // No return - stack code is now activated only through SVC and plain interrupts.
        ".align\t\n"
    );
}
#else
#error Compiler not supported.
#endif


void bootloader_util_app_start(uint32_t start_addr)
{
    StartApplication(start_addr);
}


void bootloader_util_settings_get(const bootloader_settings_t ** pp_bootloader_settings)
{
    // Read only pointer to bootloader settings in flash. 
    static bootloader_settings_t const * const p_bootloader_settings = 
        (bootloader_settings_t *)&m_boot_settings[0];        

    *pp_bootloader_settings = p_bootloader_settings;
}
