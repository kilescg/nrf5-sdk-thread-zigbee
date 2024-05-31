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

#ifndef ZB_ZCL_BASIC_ADDONS_H__
#define ZB_ZCL_BASIC_ADDONS_H__

#include "zboss_api.h"

/*! \addtogroup zb_zcl_basic_addons */
/*! @{ */

/** @brief Basic cluster attributes according to ZCL Spec 3.2.2.2 */
typedef struct
{
    zb_uint8_t zcl_version;
    zb_uint8_t app_version;
    zb_uint8_t stack_version;
    zb_uint8_t hw_version;
    zb_char_t  mf_name[33];
    zb_char_t  model_id[33];
    zb_char_t  date_code[17];
    zb_uint8_t power_source;
    zb_char_t  location_id[17];
    zb_uint8_t ph_env;
    zb_char_t  sw_ver[17];
} zb_zcl_basic_attrs_ext_t;

/**
 *  @brief Declare attribute list for Basic cluster (client).
 *  @param attr_list - attribute list name.
 */
#define ZB_ZCL_DECLARE_BASIC_CLIENT_ATTRIB_LIST(attr_list)                     \
    ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_BASIC) \
    ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

/**
 *  @brief Declare attribute list for Basic cluster (server).
 *  @param attr_list - attribute list name
 *  @param zcl_version - pointer to variable to store zcl version  attribute value
 *  @param power_source - pointer to variable to store power source attribute value
 */
#define ZB_ZCL_DECLARE_BASIC_SERVER_ATTRIB_LIST(attr_list, zcl_version, power_source) \
    ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_BASIC)        \
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID, (zcl_version))             \
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID, (power_source))           \
    ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

/** @} */

#endif /* ZB_ZCL_BASIC_ADDONS_H__ */
