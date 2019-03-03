/*
 * Copyright(c) 2019 - Jim Newman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:

 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdbool.h>
#include <stdint.h>

#include "sdk_common.h"
#include "nrf_esb.h"
#include "nrf.h"
#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "boards.h"

#include "uart.h"

volatile bool tx_mode_flag = false;
volatile bool rx_mode_flag = false;

static nrf_esb_payload_t  tx_payload;
static nrf_esb_payload_t  rx_payload;
static nrf_esb_config_t   nrf_esb_config = NRF_ESB_DEFAULT_CONFIG;

static uint32_t scope_trig = 20;

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
            printf("TX SUCCESS EVENT\n\r");
            rx_mode_flag = true;
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            printf("TX FAILED EVENT\n\r");
            tx_mode_flag = true;
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
            printf("RX RECEIVED EVENT\n\r");
            if (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
            {
                printf("Received %d Bytes\n\r", rx_payload.length);
                nrf_gpio_pin_clear(scope_trig);
                nrf_gpio_pin_set(LED_RGB_GREEN);
                nrf_gpio_pin_clear(LED_RGB_BLUE);
            }
            memcpy(&tx_payload, &rx_payload, sizeof(tx_payload));
            tx_mode_flag = true;
            break;
    }
}


void clocks_start( void )
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;

    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
}


void gpio_init( void )
{
    bsp_board_leds_init();
    nrf_gpio_cfg_output(scope_trig);
}


uint32_t esb_init(void)
{
    uint32_t err_code;
    uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
    uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };
    nrf_esb_config.tx_mode            = NRF_ESB_TXMODE_AUTO;
    nrf_esb_config.payload_length     = NRF_ESB_MAX_PAYLOAD_LENGTH;
    nrf_esb_config.retransmit_delay   = 1000;
    nrf_esb_config.retransmit_count   = 32;
    nrf_esb_config.protocol           = NRF_ESB_PROTOCOL_ESB_DPL;
    nrf_esb_config.bitrate            = NRF_ESB_BITRATE_2MBPS;
    nrf_esb_config.mode               = NRF_ESB_MODE_PRX;
    nrf_esb_config.event_handler      = nrf_esb_event_handler;
    nrf_esb_config.crc                = NRF_ESB_CRC_16BIT;
    nrf_esb_config.selective_auto_ack = false;

    err_code = nrf_esb_init(&nrf_esb_config);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_0(base_addr_0);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_1(base_addr_1);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_prefixes(addr_prefix, 8);
    VERIFY_SUCCESS(err_code);

    return err_code;
}


int main(void)
{
    uint32_t err_code;

    gpio_init();
    uart_init();

    clocks_start();
    
    nrf_gpio_pin_set(LED_RGB_RED);
    nrf_gpio_pin_set(LED_RGB_GREEN);
    nrf_gpio_pin_set(LED_RGB_BLUE);

    tx_payload.pipe = 0;
    tx_payload.length = NRF_ESB_MAX_PAYLOAD_LENGTH;

    err_code = esb_init();
    APP_ERROR_CHECK(err_code);

    printf("Enhanced ShockBurst Transceiver Running.\n\r");
    printf("Transmitting a Maximum of %d Bytes Per Packet.\n\r", tx_payload.length);

    err_code = nrf_esb_start_rx();
    APP_ERROR_CHECK(err_code);

    while (true)
    {
        if (tx_mode_flag == true)
        {
#ifdef DEBUG_XFER
            printf("RX Payload\n\r");
            for (int i = 0; i < rx_payload.length; i++)
            {
                printf(" 0x%02x", rx_payload.data[i]);
                if (i % 16 == 15)
                    printf("\n\r");
            }
            if (rx_payload.length % 16 != 0)
                printf("\n\r");
            printf("TX Payload\n\r");
            for (int i = 0; i < tx_payload.length; i++)
            {
                printf(" 0x%02x", tx_payload.data[i]);
                if (i % 16 == 15)
                    printf("\n\r");
            }
            if (tx_payload.length % 16 != 0)
                printf("\n\r");
#endif            
            tx_mode_flag = false;
            nrf_esb_stop_rx();
            nrf_esb_disable();
            nrf_esb_config.mode = NRF_ESB_MODE_PTX;
            nrf_esb_init(&nrf_esb_config);
            nrf_esb_start_tx();
            //printf("Transmitting packet\n\r");
            tx_payload.noack = false;
            nrf_gpio_pin_set(scope_trig);
            nrf_gpio_pin_clear(LED_RGB_GREEN);
            nrf_gpio_pin_set(LED_RGB_BLUE);
            if (!(nrf_esb_write_payload(&tx_payload) == NRF_SUCCESS))
            {
                printf("Sending packet failed\n\r");
                rx_mode_flag = true;
            }
        }
        else if (rx_mode_flag == true)
        {
            rx_mode_flag = false;
            nrf_esb_disable();
            nrf_esb_config.mode = NRF_ESB_MODE_PRX;
            nrf_esb_init(&nrf_esb_config);
            nrf_esb_start_rx();
        }
        //nrf_delay_us(50000);
        //__WFE();
    }
}
