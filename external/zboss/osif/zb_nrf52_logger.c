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
#include "sdk_config.h"
#include <drivers/nrfx_common.h>

void nrf_logger_init(void);
void nrf_serial_logger_init(void);

void nrf_logger_msg_port_do(void);
void nrf_serial_logger_msg_port_do(void);

void nrf_logger_put_bytes(const zb_uint8_t *, zb_short_t);
void nrf_serial_logger_put_bytes(const zb_uint8_t *, zb_short_t);

void nrf_logger_flush(void);
void nrf_serial_logger_flush(void);

void nrf_logger_set_rx_cb(zb_callback_t cb);
void nrf_serial_logger_set_rx_cb(zb_callback_t cb);


void zb_osif_serial_init(void)
{
#if defined(CONFIG_ZBOSS_TRACE_BINARY_LOGGING)
    nrf_serial_logger_init();
#else
    nrf_logger_init();
#endif
}

void zb_osif_serial_put_bytes(const zb_uint8_t *bytes, zb_short_t len)
{
#if defined(CONFIG_ZBOSS_TRACE_BINARY_LOGGING)
    nrf_serial_logger_put_bytes(bytes, len);
#else
    nrf_logger_put_bytes(bytes, len);
#endif
}

void zb_osif_serial_flush(void)
{
#if defined(CONFIG_ZBOSS_TRACE_BINARY_LOGGING)
    nrf_serial_logger_flush();
#else
    nrf_logger_flush();
#endif
}

#ifdef CONFIG_ZB_NRF_TRACE_RX_ENABLE
void zb_osif_set_uart_byte_received_cb(zb_callback_t cb)
{
#if defined(CONFIG_ZBOSS_TRACE_BINARY_LOGGING)
    nrf_serial_logger_set_rx_cb(cb);
#else
    nrf_logger_set_rx_cb(cb);
#endif
}
#endif

void zb_trace_msg_port_do(void)
{
#if defined(CONFIG_ZBOSS_TRACE_BINARY_LOGGING)
    nrf_serial_logger_msg_port_do();
#else
    nrf_logger_msg_port_do();
#endif
}
