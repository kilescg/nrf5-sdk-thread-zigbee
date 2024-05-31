/**
 * Copyright (c) 2021 - 2022, Nordic Semiconductor ASA
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
#ifndef ZIGBEE_STATISTICS_H__
#define ZIGBEE_STATISTICS_H__

#include <stdint.h>
#include <stdbool.h>

#include "zboss_api.h"

#if defined ZIGBEE_NRF_RADIO_STATISTICS
typedef struct zigbee_nrf_radio_stats_s
{
  zb_uint32_t rx_successful;
  zb_uint32_t rx_err_none; /* Error Code: 0x00 */
  zb_uint32_t rx_err_invalid_frame; /* Error Code: 0x01 */
  zb_uint32_t rx_err_invalid_fcs; /* Error Code: 0x02 */
  zb_uint32_t rx_err_invalid_dest_addr; /* Error Code: 0x03 */
  zb_uint32_t rx_err_runtime; /* Error Code: 0x04 */
  zb_uint32_t rx_err_timeslot_ended; /* Error Code: 0x05 */
  zb_uint32_t rx_err_aborted; /* Error Code: 0x06 */

  zb_uint32_t tx_successful;
  zb_uint32_t tx_err_none; /* Error Code: 0x00 */
  zb_uint32_t tx_err_busy_channel; /* Error Code: 0x01 */
  zb_uint32_t tx_err_invalid_ack; /* Error Code: 0x02 */
  zb_uint32_t tx_err_no_mem; /* Error Code: 0x03 */
  zb_uint32_t tx_err_timeslot_ended; /* Error Code: 0x04 */
  zb_uint32_t tx_err_no_ack; /* Error Code: 0x05 */
  zb_uint32_t tx_err_aborted; /* Error Code: 0x06 */
  zb_uint32_t tx_err_timeslot_denied; /* Error Code: 0x07 */

} zigbee_nrf_radio_stats_t;

zigbee_nrf_radio_stats_t* zigbee_get_nrf_radio_stats(void);
#endif /* defined ZIGBEE_NRF_RADIO_STATISTICS */

#endif /* ZIGBEE_STATISTICS_H__ */
