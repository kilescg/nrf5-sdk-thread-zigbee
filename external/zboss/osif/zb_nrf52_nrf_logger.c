/* ZBOSS Zigbee 3.0
 *
 * Copyright (c) 2012-2018 DSR Corporation, Denver CO, USA.
 * http://www.dsr-zboss.com
 * http://www.dsr-corporation.com
 * All rights reserved.
 *
 *
 * Use in source and binary forms, redistribution in binary form only, with
 * or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 3. This software, with or without modification, must only be used with a Nordic
 *    Semiconductor ASA integrated circuit.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
PURPOSE: Platform specific for NRF52 SoC.
*/

#define ZB_TRACE_FILE_ID 36508
#include "zboss_api.h"
#include "zb_nrf52_internal.h"


#if defined ZB_NRF_TRACE
/* NRF Log subsystem */

#define NRF_LOG_MODULE_NAME zboss
#define TRACE_LOG_BUFFER_SIZE 80U

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "nrf_log.h"

NRF_LOG_MODULE_REGISTER();
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static zb_uint8_t buf8[TRACE_LOG_BUFFER_SIZE];
static zb_uint_t buffered = 0;
static zb_bool_t m_initialized = ZB_FALSE;

void nrf_logger_init(void)
{
#ifdef ZB_ASYNC_TRACE_CONTROL
  static zb_bool_t backend_inited = ZB_FALSE;
#endif

  if ((m_initialized == ZB_FALSE) && (
#ifdef ZB_TRACE_LEVEL
        (g_trace_level != 0)
#else
        0
#endif
        ||
#ifdef ZB_TRAFFIC_DUMP_ON
        (g_traf_dump != 0)
#else
        0
#endif
        ))
  {
#ifdef ZB_ASYNC_TRACE_CONTROL
    if (!backend_inited)
    {
      APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
      NRF_LOG_DEFAULT_BACKENDS_INIT();
      backend_inited = ZB_TRUE;
    }
#endif
    m_initialized = ZB_TRUE;
  }
}

/* Is called when complete Trace message is put.
 * Triggers sending buffered data.
 */
void nrf_logger_msg_port_do(void)
{
  if (buffered)
  {
    NRF_LOG_HEXDUMP_INFO(buf8, buffered);
    buffered = 0;
  }
}

void nrf_logger_put_bytes(const zb_uint8_t *buf, zb_short_t len)
{
  zb_short_t bytes_copied = 0;
  zb_uint8_t bytes_to_copy = 0;

  if (m_initialized == ZB_FALSE)
  {
    return;
  }

  while (len)
  {
    zb_uint8_t buffer_free_space = (TRACE_LOG_BUFFER_SIZE - buffered);

    /* Send what is in buffer to free space. */
    if (buffer_free_space == 0)
    {
      nrf_logger_msg_port_do();
      continue;
    }

    /* Copy all bytes buffer can store. */
    if (len > buffer_free_space) {
      bytes_to_copy = buffer_free_space;
    }
    else
    {
      bytes_to_copy = len;
    }

    ZB_MEMCPY((buf8 + buffered), (buf + bytes_copied), bytes_to_copy);
    buffered += bytes_to_copy;
    bytes_copied += bytes_to_copy;
    len -= bytes_to_copy;
  }
}

/*Function set UART RX callback function*/
void nrf_logger_set_rx_cb(zb_callback_t cb)
{
  /* Not supported */
}

void nrf_logger_flush(void)
{
  nrf_logger_msg_port_do();
  UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
}

#endif /* defined ZB_NRF_TRACE */
