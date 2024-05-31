/**
 * Copyright (c) 2018 - 2021, Nordic Semiconductor ASA
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

#ifndef ZB_MACLL_H__
#define ZB_MACLL_H__

#include <stdint.h>
#include "zb_transceiver.h"

/** The number of metadata bytes, attached to each received frame,
 *  at the end of ZBOSS buffer.
 */
#define ZB_MACLL_EXTRA_DATA_SIZE     sizeof(zb_macll_metadata_t)

/** The number of bytes used to pass the frame length field,
 *  at the beginning of ZBOSS buffer.
 */
#define ZB_MACLL_PACKET_LENGTH_SIZE  1

/**
   Get a reference to MAC LL specific metadata structure.
   @param packet - pointer to buffer
*/
#define ZB_MACLL_GET_METADATA(packet)  ((zb_macll_metadata_t *)((zb_uint8_t*)zb_buf_end(packet) - ZB_MACLL_EXTRA_DATA_SIZE))

/** Status code indicating that the received Data frame
 *  was not acknowledged with a pending bit set (indirect poll).
 *
 * That status should not be set directly by the transceiver implementation,
 * but the zb_macll_set_received_data_status(..) API should be used.
 */
#define ZB_MACLL_NO_DATA 0xeb


/** Structure attached to each ZBOSS buffer used to pass additional metadata. */
typedef struct
{
    uint8_t lqi;
    int8_t  power;
} ZB_PACKED_STRUCT zb_macll_metadata_t;


void zb_macll_init(void);

#define zb_macll_set_trans_int()      /* Transceiver interrupt is a logical or of the TX and RX interrupt flag. */
#define zb_macll_clear_trans_int()    /* Transceiver interrupt is a logical or of the TX and RX interrupt flag. */
#define zb_macll_get_trans_int()      (zb_macll_get_rx_flag() || zb_macll_get_tx_flag())

#define zb_macll_set_rx_flag()        /* Implemented through reading RX status via zb_trans_rx_pending(). */
#define zb_macll_clear_rx_flag()      /* Implemented through reading RX status via zb_trans_rx_pending(). */
#define zb_macll_get_rx_flag          zb_trans_rx_pending

void zb_macll_set_tx_flag(void);
void zb_macll_clear_tx_flag(void);
zb_bool_t zb_macll_get_tx_flag(void);

zb_uint8_t zb_macll_src_match_set_pending_bit(zb_uint8_t *addr, zb_bool_t value, zb_bool_t extended);
#define zb_macll_src_match_tbl_drop   zb_trans_src_match_tbl_drop

#define zb_macll_set_short_addr       zb_trans_set_short_addr
#define zb_macll_set_long_addr        zb_trans_set_long_addr
#define zb_macll_set_pan_id           zb_trans_set_pan_id
#define zb_macll_set_pan_coord        zb_trans_set_pan_coord

#define zb_macll_get_radio_on_off     zb_trans_is_active
#define zb_macll_set_tx_power         zb_trans_set_tx_power
#define zb_macll_get_tx_power         zb_trans_get_tx_power
#define zb_macll_set_channel          zb_trans_set_channel
#define zb_macll_start_get_rssi       zb_trans_start_get_rssi
#define zb_macll_get_rssi             zb_trans_get_rssi

void zb_macll_send_packet(zb_bufid_t buf, zb_uint8_t wait_type);
void zb_macll_resend_packet(zb_uint8_t wait_type, zb_time_t tx_at);
zb_time_t zb_macll_get_tx_timestamp(void);

void zb_macll_transmitted_raw(zb_uint8_t * p_ack);
void zb_macll_transmit_failed(zb_uint8_t error);

void zb_macll_trans_set_rx_on_off(int rx_on);
#define zb_macll_get_rx_on_off        zb_trans_is_receiving
/**@brief Pass to MAC data packet already read by radio driver layer.
 *
 * @param buf - ZBOSS buffer - placeholder for incoming data.
 *
 * @retval[0]  No packets available.
 * @retval[1]  New packet copied to the buf.
 */
#define zb_macll_get_next_packet      zb_trans_get_next_packet

int8_t zb_macll_metadata_get_power(zb_bufid_t bufid);
void zb_macll_metadata_set_power(zb_bufid_t bufid, int8_t power);
uint8_t zb_macll_metadata_get_lqi(zb_bufid_t bufid);
void zb_macll_metadata_set_lqi(zb_bufid_t bufid, uint8_t lqi);
void zb_macll_set_received_data_status(zb_bufid_t bufid, zb_bool_t pending_bit);

zb_ret_t zb_macll_tx_carrier(zb_uint8_t channel, zb_time_t timeout_bi);

#endif /* ZB_MACLL_H__ */
