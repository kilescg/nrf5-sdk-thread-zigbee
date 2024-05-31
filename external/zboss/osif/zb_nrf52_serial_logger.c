/**
 * Copyright (c) 2022, Nordic Semiconductor ASA
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

#include "zboss_api.h"
#include "nrf_drv_uart.h"
#include "nrf_ringbuf.h"
#include "sdk_errors.h"
#include "nrf_atomic.h"

#include "boards.h"
#include "sdk_config.h"


#define SERIAL_RX_CHUNK_SIZE 1

NRF_RINGBUF_DEF(uart_tx_buf, CONFIG_ZBOSS_TRACE_LOGGER_BUF_SIZE);

static uint8_t uart_rx_buf[SERIAL_RX_CHUNK_SIZE];
static nrf_drv_uart_t uart_inst = NRF_DRV_UART_INSTANCE(0);
static zb_bool_t initialized;
static zb_callback_t rx_callback;
static nrf_atomic_flag_t stop_tx_req = 0;
static nrf_atomic_u32_t tx_bytes_free = 0;

static void uart_event_tx_done_handler(size_t bytes_sent)
{
    zb_uint8_t *data = NULL;
    size_t data_len = UARTE0_EASYDMA_MAXCNT_SIZE;
    ret_code_t ret;

    if (stop_tx_req)
    {
        UNUSED_RETURN_VALUE(nrf_atomic_u32_store(&tx_bytes_free, bytes_sent));
        return;
    }

    ret = nrf_ringbuf_free(&uart_tx_buf, bytes_sent);
    APP_ERROR_CHECK(ret);

    // Check ring buffer - retrigger transfer if ring buffer not empty
    ret = nrf_ringbuf_get(&uart_tx_buf, &data, &data_len, false);
    APP_ERROR_CHECK(ret);

    if (data_len != 0)
    {
        ret = nrf_drv_uart_tx(&uart_inst, data, data_len);
        APP_ERROR_CHECK(ret);
    }
}

static void uart_event_rx_done_handler(uint8_t *data, uint8_t bytes)
{
    uint8_t i;

    if (!rx_callback)
    {
        return;
    }

    for (i = 0; i < bytes; i++)
    {
        rx_callback(data[i]);
    }
}

static void uart_event_error_handler(void)
{

}

static void uart_event_handler(nrf_drv_uart_event_t *event, void *ctx)
{
    ret_code_t ret;

    if (event == NULL)
    {
        return;
    }

    switch(event->type)
    {
        case NRF_DRV_UART_EVT_TX_DONE:
            uart_event_tx_done_handler(event->data.rxtx.bytes);
            break;
        case NRF_DRV_UART_EVT_RX_DONE:
            if (event->data.rxtx.bytes)
            {
                uart_event_rx_done_handler(event->data.rxtx.p_data, event->data.rxtx.bytes);
            }
            ret = nrf_drv_uart_rx(&uart_inst, uart_rx_buf, SERIAL_RX_CHUNK_SIZE);
            APP_ERROR_CHECK(ret);
            break;
        case NRF_DRV_UART_EVT_ERROR:
            ret = nrf_drv_uart_rx(&uart_inst, uart_rx_buf, SERIAL_RX_CHUNK_SIZE);
            APP_ERROR_CHECK(ret);
            uart_event_error_handler();
            break;
        default:
            break;
    }
}

void nrf_serial_logger_init(void)
{
    nrf_drv_uart_config_t config = NRF_DRV_UART_DEFAULT_CONFIG;
    ret_code_t ret;

    if (initialized)
    {
        return;
    }

    nrf_ringbuf_init(&uart_tx_buf);

#if defined(TX_PIN_NUMBER)
    config.pseltxd = TX_PIN_NUMBER;
#endif

#if defined(RX_PIN_NUMBER)
    config.pselrxd = RX_PIN_NUMBER;
#endif

#if defined(HWFC) && (HWFC == true)
    config.hwfc = NRF_UART_HWFC_ENABLED;

#if defined(CTS_PIN_NUMBER)
    config.pselcts = CTS_PIN_NUMBER;
#endif

#if defined(RTS_PIN_NUMBER)
    config.pselrts = RTS_PIN_NUMBER;
#endif

#endif /* defined(HWFC) && (HWFC == true) */

    ret = nrf_drv_uart_init(&uart_inst, &config, uart_event_handler);
    if (ret != NRF_SUCCESS)
    {
        return;
    }

#ifdef CONFIG_ZB_NRF_TRACE_RX_ENABLE
    ret = nrf_drv_uart_rx(&uart_inst, uart_rx_buf, SERIAL_RX_CHUNK_SIZE);
    if (ret != NRF_SUCCESS)
    {
        return;
    }
#endif

    initialized = ZB_TRUE;
}

void nrf_serial_logger_msg_port_do(void)
{
    zb_uint8_t *data = NULL;
    size_t data_len = UARTE0_EASYDMA_MAXCNT_SIZE;
    ret_code_t ret;

    if (!initialized)
    {
        return;
    }

    // Trigger UART transfer only if not already in progress.
    if (nrf_drv_uart_tx_in_progress(&uart_inst))
    {
        return;
    }

    ret = nrf_ringbuf_get(&uart_tx_buf, &data, &data_len, false);
    APP_ERROR_CHECK(ret);

    if (data_len != 0)
    {
        ret = nrf_drv_uart_tx(&uart_inst, data, data_len);
        APP_ERROR_CHECK(ret);
    }
}

void nrf_serial_logger_put_bytes(const zb_uint8_t *bytes, zb_short_t nbytes)
{
    size_t nbytes_put = nbytes;
    ret_code_t ret;

    if (!initialized)
    {
        return;
    }

    while (nbytes)
    {
        UNUSED_RETURN_VALUE(nrf_atomic_flag_set(&stop_tx_req));

        ret = nrf_ringbuf_cpy_put(&uart_tx_buf, bytes, &nbytes_put);

        UNUSED_RETURN_VALUE(nrf_atomic_flag_clear(&stop_tx_req));

        APP_ERROR_CHECK(ret);

        bytes += nbytes_put;
        nbytes -= nbytes_put;
        nbytes_put = nbytes;

        if (tx_bytes_free != 0)
        {
            ret = nrf_ringbuf_free(&uart_tx_buf, tx_bytes_free);
            APP_ERROR_CHECK(ret);

            // Don't trigger UART transfer if enough memory was recently freed
            if (tx_bytes_free >= (uint32_t)nbytes)
            {
                UNUSED_RETURN_VALUE(nrf_atomic_u32_store(&tx_bytes_free, 0));
                continue;
            }
            UNUSED_RETURN_VALUE(nrf_atomic_u32_store(&tx_bytes_free, 0));
        }

        if (nbytes)
        {
            nrf_serial_logger_msg_port_do();
        }
    }
}

void nrf_serial_logger_flush(void)
{
    bool tx_busy = true;

    // Wait until previous transaction completes
    while (nrf_drv_uart_tx_in_progress(&uart_inst));

    while (tx_busy)
    {
        nrf_serial_logger_msg_port_do();
        tx_busy = nrf_drv_uart_tx_in_progress(&uart_inst);
        while (nrf_drv_uart_tx_in_progress(&uart_inst));
    }
}

void nrf_serial_logger_set_rx_cb(zb_callback_t cb)
{
    rx_callback = cb;
}
