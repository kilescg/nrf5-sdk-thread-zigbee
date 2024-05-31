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

#ifndef ZB_TRANSCEIVER_H__
#define ZB_TRANSCEIVER_H__

void zb_trans_hw_init(void);
void zb_trans_set_pan_id(zb_uint16_t pan_id);
void zb_trans_set_long_addr(zb_ieee_addr_t long_addr);
void zb_trans_set_short_addr(zb_uint16_t addr);
void zb_trans_start_get_rssi(zb_uint8_t scan_duration_bi);
void zb_trans_get_rssi(zb_uint8_t *rssi_value_p);
void zb_trans_set_channel(zb_uint8_t channel_number);
void zb_trans_set_tx_power(zb_int8_t power);
void zb_trans_get_tx_power(zb_int8_t *power);
void zb_trans_set_pan_coord(zb_bool_t pan_coord);
void zb_trans_set_auto_ack(zb_bool_t enabled);
void zb_trans_set_promiscuous_mode(zb_bool_t enabled);
void zb_trans_enter_receive(void);
void zb_trans_enter_sleep(void);
zb_bool_t zb_trans_is_receiving(void);
zb_bool_t zb_trans_is_active(void);
zb_bool_t zb_trans_transmit(zb_uint8_t wait_type, zb_time_t tx_at, zb_uint8_t *tx_buf, zb_uint8_t current_channel);
void zb_trans_buffer_free(zb_uint8_t *p_buf);
zb_bool_t zb_trans_set_pending_bit(zb_uint8_t *addr, zb_bool_t value, zb_bool_t extended);
void zb_trans_src_match_tbl_drop(void);
zb_time_t osif_sub_trans_timer(zb_time_t t2, zb_time_t t1);
zb_bool_t zb_trans_rx_pending(void);
zb_uint8_t zb_trans_get_next_packet(zb_bufid_t buf);
zb_ret_t zb_trans_cca(void);
zb_ret_t zb_trans_continuous_carrier(void);
void zb_trans_set_crcpoly(zb_uint32_t iv, zb_uint32_t polynomial);

#endif /* ZB_TRANSCEIVER_H__ */
