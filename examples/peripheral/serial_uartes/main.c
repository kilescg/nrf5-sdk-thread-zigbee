/**
 * Copyright (c) 2018 - 2022, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "nrf.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_drv_power.h"
#include "nrf_serial.h"
#include "app_timer.h"


#include "app_error.h"
#include "app_util.h"
#include "boards.h"

/** @file
 * @defgroup nrf_serial_uartes_example main.c
 * @{
 * @ingroup nrf_serial_uartes_example
 * @brief Example of @ref nrf_serial usage. Loopback example using two UARTE peripherals.
 *        Please short Arduino SCL and SDA GPIOs to start transmission.
 *
 */

#define OP_QUEUES_SIZE          3
#define APP_TIMER_PRESCALER     NRF_SERIAL_APP_TIMER_PRESCALER

static void sleep_handler(void)
{
    __WFE();
    __SEV();
    __WFE();
}

NRF_SERIAL_DRV_UART_CONFIG_DEF(m_uarte0_drv_config,
                      RX_PIN_NUMBER, ARDUINO_SCL_PIN,
                      RTS_PIN_NUMBER, CTS_PIN_NUMBER,
                      NRF_UART_HWFC_DISABLED, NRF_UART_PARITY_EXCLUDED,
                      NRF_UART_BAUDRATE_115200,
                      UART_DEFAULT_CONFIG_IRQ_PRIORITY);

NRF_SERIAL_DRV_UART_CONFIG_DEF(m_uarte1_drv_config,
                      ARDUINO_SDA_PIN, TX_PIN_NUMBER,
                      RTS_PIN_NUMBER, CTS_PIN_NUMBER,
                      NRF_UART_HWFC_DISABLED, NRF_UART_PARITY_EXCLUDED,
                      NRF_UART_BAUDRATE_115200,
                      UART_DEFAULT_CONFIG_IRQ_PRIORITY);


#define SERIAL_FIFO_TX_SIZE 32
#define SERIAL_FIFO_RX_SIZE 32

NRF_SERIAL_QUEUES_DEF(serial0_queues, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);
NRF_SERIAL_QUEUES_DEF(serial1_queues, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);


#define SERIAL_BUFF_TX_SIZE 1
#define SERIAL_BUFF_RX_SIZE 1

NRF_SERIAL_BUFFERS_DEF(serial0_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);
NRF_SERIAL_BUFFERS_DEF(serial1_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);


NRF_SERIAL_CONFIG_DEF(serial0_config, NRF_SERIAL_MODE_DMA,
                      &serial0_queues, &serial0_buffs, NULL, sleep_handler);
NRF_SERIAL_CONFIG_DEF(serial1_config, NRF_SERIAL_MODE_DMA,
                      &serial1_queues, &serial1_buffs, NULL, sleep_handler);


NRF_SERIAL_UART_DEF(serial0_uarte, 0);
NRF_SERIAL_UART_DEF(serial1_uarte, 1);


int main(void)
{
    ret_code_t ret;

    ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);
    ret = nrf_drv_power_init(NULL);
    APP_ERROR_CHECK(ret);

    nrf_drv_clock_lfclk_request(NULL);
    ret = app_timer_init();
    APP_ERROR_CHECK(ret);

    // Initialize LEDs and buttons.
    bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);

    ret = nrf_serial_init(&serial0_uarte, &m_uarte0_drv_config, &serial0_config);
    APP_ERROR_CHECK(ret);

    ret = nrf_serial_init(&serial1_uarte, &m_uarte1_drv_config, &serial1_config);
    APP_ERROR_CHECK(ret);

    static char tx_message[] = "Hello nrf_serial!\n\r";

    ret = nrf_serial_write(&serial1_uarte,
                           tx_message,
                           strlen(tx_message),
                           NULL,
                           NRF_SERIAL_MAX_TIMEOUT);
    (void)nrf_serial_flush(&serial1_uarte, 0);

    while (true)
    {
        char c;
        ret = nrf_serial_read(&serial0_uarte, &c, sizeof(c), NULL, 1000);
        if (ret != NRF_SUCCESS)
        {
            continue;
        }
        (void)nrf_serial_write(&serial0_uarte, &c, sizeof(c), NULL, 0);
        (void)nrf_serial_flush(&serial0_uarte, 0);

        ret = nrf_serial_read(&serial1_uarte, &c, sizeof(c), NULL, 1000);
        if (ret != NRF_SUCCESS)
        {
            continue;
        }
        (void)nrf_serial_write(&serial1_uarte, &c, sizeof(c), NULL, 0);
        (void)nrf_serial_flush(&serial1_uarte, 0);
    }
}

/** @} */
