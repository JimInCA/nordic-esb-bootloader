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

#include "uart.h"
#include "includes.h"
#include "app_fifo.h"
#include "nrf_delay.h"
#include "boards.h"

#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */

//#define ALTERNATE_UART_PINS
#ifdef ALTERNATE_UART_PINS
    #define UART_BAUDRATE     UART_BAUDRATE_BAUDRATE_Baud115200
    #undef RX_PIN_NUMBER
    #define RX_PIN_NUMBER  20
    #undef TX_PIN_NUMBER
    #define TX_PIN_NUMBER  19
    #undef RTS_PIN_NUMBER
    #define RTS_PIN_NUMBER 18
    #undef CTS_PIN_NUMBER
    #define CTS_PIN_NUMBER 17
#else
    #define UART_BAUDRATE     UART_BAUDRATE_BAUDRATE_Baud115200
#endif

const uint8_t tag[4] = {'L', 'E', 'A', 'P'};

void uart_evt_handle(app_uart_evt_t * p_event)
{
    uint32_t err_code;
    uint8_t ch;
    static uint8_t index = 0;

    if (p_event->evt_type == APP_UART_DATA_READY)
    {
        err_code = uart_get(&ch);
        if (err_code == NRF_SUCCESS)
        {
            switch (index)
            {
            case 0:
            case 1:
            case 2:
            case 3: if (ch == tag[index])
                    {
                        tx_payload.data[index] = ch;
                        index++;
                    }
                    else
                        index = 0;
                    break;
            case 4:
            case 5: tx_payload.data[index] = ch;
                    index++;
                    break;
            default:
                    tx_payload.data[index] = ch;
                    if (index == tx_payload.data[4]+5)
                    {
                         tx_payload.length = index+1;
                         TX_MODE_FLAG = true;
                         index = 0;
                     }
                     else
                         index++;
                     break;
            }
            if (index > NRF_ESB_MAX_PAYLOAD_LENGTH)
                index = 0;
        }
    }
    else if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        nrf_gpio_pin_clear(LED_RGB_RED);
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        nrf_gpio_pin_clear(LED_RGB_RED);
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

void uart_init(void)
{
    uint32_t err_code;

    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          UART_BAUDRATE
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_evt_handle,
                         APP_IRQ_PRIORITY_LOW,
                         err_code);

    APP_ERROR_CHECK(err_code);
    uart_flush();
}

void uart_send(uint8_t *buffer, uint8_t size)
{
    uint32_t err_code;
    nrf_gpio_pin_set(scope_trig);
    for (int i = 0; i < size; i++)
    {
        do
        {
    		nrf_gpio_pin_toggle(scope_trig);
            err_code = app_uart_put(buffer[i]);
            if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
            {
                nrf_gpio_pin_clear(LED_RGB_RED);
            }
        } while (err_code == NRF_ERROR_BUSY);
    }
    nrf_gpio_pin_clear(scope_trig);
}

