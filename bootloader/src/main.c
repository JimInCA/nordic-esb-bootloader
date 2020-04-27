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

/**@file
 *
 * @defgroup bootloader main.c
 * @{
 * @ingroup dfu_bootloader_api
 * @brief Bootloader project main file.
 *
 * -# Receive start data package. 
 * -# Based on start packet, prepare NVM area to store received data. 
 * -# Receive data packet. 
 * -# Validate data packet.
 * -# Write Data packet to NVM.
 * -# If not finished - Wait for next packet.
 * -# Receive stop data packet.
 * -# Activate Image, boot application.
 *
 */
 
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "dfu.h"
#include "bootloader.h"
#include "app_timer.h"
#include "app_gpiote.h"
#include "boards.h"
#include "nrf_delay.h"

#define RED   0
#define GREEN 1
#define BLUE  2

#define BOOTLOADER_GPIO_PIN             15                  /**< GPIO used to enter SW update mode. */

#define APP_GPIOTE_MAX_USERS            2                   /**< Number of GPIOTE users in total. Used by button module and dfu_transport_serial module (flow control). */

#define APP_TIMER_PRESCALER             0                   /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS            3                   /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE         4                   /**< Size of timer operation queues. */

#if defined ( __CC_ARM )
#define ALLIGN32 __align(4)
#elif defined ( __GNUC__ )
#define ALLIGN32 __attribute__((aligned (32)))
#else
#error Compiler not supported.
#endif

// the following variable must be aligned on a 4-byte boundry to prevent a hard fault in accessing it from BranchIntHandler()
ALLIGN32 uint8_t bootloader_active = 1;

/**@brief Function for error handling, which is called when an error has occurred. 
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze 
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name. 
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    nrf_gpio_pin_set(BLUE);

    // On assert, the system can only recover on reset.
    NVIC_SystemReset();
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze 
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] file_name   File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}


/**@brief Function for initializing the GPIOTE handler module.
 */
static void gpiote_init(void)
{
    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    uint32_t err_code;
    // Initialize timer module, making it use the scheduler.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the gpio module.
 */
static void gpio_init(void)
{
    nrf_gpio_cfg_input(BOOTLOADER_GPIO_PIN, NRF_GPIO_PIN_PULLUP);
}


/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    
    bool bootloader_is_pushed = false;
    
    // initialize
    bsp_board_init(BSP_INIT_LEDS);
    timers_init();
    gpiote_init();
    gpio_init();

    // This check ensures that the defined fields in the bootloader corresponds with actual
    // setting in the nRF51 chip.
    //APP_ERROR_CHECK_BOOL(NRF_UICR->CLENR0 == CODE_REGION_1_START);

    //APP_ERROR_CHECK_BOOL(*((uint32_t *)NRF_UICR_BOOT_START_ADDRESS) == BOOTLOADER_REGION_START);
    APP_ERROR_CHECK_BOOL(NRF_FICR->CODEPAGESIZE == CODE_PAGE_SIZE);
    
    // allow input pin to stablize prior to read
    nrf_delay_us(100);
    bootloader_is_pushed = nrf_gpio_pin_read(BOOTLOADER_GPIO_PIN) ? false:true;  // need to flip state
    
    //bsp_board_led_on(bootloader_is_pushed ? BLUE : RED);
    //nrf_delay_us(1000000L);
    
    if (bootloader_is_pushed || (!bootloader_app_is_valid(DFU_BANK_0_REGION_START)))
    {
        bsp_board_led_on(RED);
        bootloader_active = 1;
        // Initiate an update of the firmware.
        err_code = bootloader_dfu_start();
        APP_ERROR_CHECK(err_code);

        bsp_board_led_off(RED);
    }

    if (bootloader_app_is_valid(DFU_BANK_0_REGION_START))
    {
        bsp_board_leds_off();
        
        // Select a bank region to use as application region.
        // @note: Only applications running from DFU_BANK_0_REGION_START is supported.
        bootloader_active = 0;
        bootloader_app_start(DFU_BANK_0_REGION_START);
    }

    bsp_board_led_off(RED);
    bsp_board_led_off(GREEN);
    bsp_board_led_off(BLUE);
    
    NVIC_SystemReset();
}
