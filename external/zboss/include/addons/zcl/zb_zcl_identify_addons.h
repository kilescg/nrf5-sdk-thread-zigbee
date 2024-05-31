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

#ifndef ZB_ZCL_IDENTIFY_ADDONS_H__
#define ZB_ZCL_IDENTIFY_ADDONS_H__

#include "zboss_api.h"

/*! \addtogroup zb_zcl_identify_addons */
/*! @{ */

/**@brief Macro equivalent to @ref ZB_ZCL_IDENTIFY_SEND_TRIGGER_VARIANT_REQ, but command is sent without APS ACK. */
#define ZB_ZCL_IDENTIFY_SEND_TRIGGER_VARIANT_REQ_NO_APS_ACK(                                        \
  buffer, addr, dst_addr_mode, dst_ep, ep, prof_id, def_resp, cb, effect_id, effect_var)            \
{                                                                                                   \
    zb_uint8_t * ptr = ZB_ZCL_START_PACKET_REQ(buffer)                                              \
    ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL(ptr, def_resp)                              \
    ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(ptr, ZB_ZCL_GET_SEQ_NUM(),                                  \
            ZB_ZCL_CMD_IDENTIFY_TRIGGER_EFFECT_ID);                                                 \
    ZB_ZCL_PACKET_PUT_DATA8(ptr, (effect_id));                                                      \
    ZB_ZCL_PACKET_PUT_DATA8(ptr, (effect_var));                                                     \
    ZB_ZCL_SEND_COMMAND_SHORT_WITHOUT_ACK((buffer), ptr, addr, dst_addr_mode,                       \
                                dst_ep, ep, prof_id, ZB_ZCL_CLUSTER_ID_IDENTIFY, cb, 0);            \
}

/**
 *  @brief Declare attribute list for Identify cluster (client).
 *  @param attr_list - attribute list name.
 */
#define ZB_ZCL_DECLARE_IDENTIFY_CLIENT_ATTRIB_LIST(attr_list)                   \
  ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_IDENTIFY) \
  ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

/**
 *  @brief Declare attribute list for Identify cluster (server).
 *  @param attr_list - attribute list name
 *  @param identify_time - pointer to variable to store identify time attribute value
 */
#define ZB_ZCL_DECLARE_IDENTIFY_SERVER_ATTRIB_LIST(attr_list, identify_time)    \
  ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_IDENTIFY) \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID, (identify_time))  \
  ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

/** @} */

#endif /* ZB_ZCL_IDENTIFY_ADDONS_H__ */
