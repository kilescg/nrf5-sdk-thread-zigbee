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
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>


#include "nrf.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"

#include "nrf_drv_power.h"

#include "app_error.h"
#include "app_util.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "boards.h"

#include "nrf_crypto.h"
#include "nrf_crypto_error.h"
#include "mem_manager.h"


#if NRF_MODULE_ENABLED(NRF_CRYPTO)
/**@file
 * @defgroup AES_CBC_MAC_example main.c
 *
 * @{
 *
 */

#define AES_MAC_SIZE                            (16)

#define NRF_CRYPTO_EXAMPLE_AES_MAX_TEXT_SIZE    (100)

#define AES_ERROR_CHECK(error)  \
    do {            \
        if (error)  \
        {           \
            NRF_LOG_RAW_INFO("\r\nError = 0x%x\r\n%s\r\n",           \
                             (error),                                \
                             nrf_crypto_error_string_get(error));    \
            return; \
        }           \
    } while (0);

/* Maximum allowed key = 256 bit */
static uint8_t m_key[32] = {'N', 'O', 'R', 'D', 'I', 'C', ' ',
                            'S', 'E', 'M', 'I', 'C', 'O', 'N', 'D', 'U', 'C', 'T', 'O', 'R',
                            'A', 'E', 'S', '&', 'M', 'A', 'C', ' ', 'T', 'E', 'S', 'T'};

/* Below text is used as plain text for MAC calculation. Its length must be multiple of 16 bytes
   to use NRF_CRYPTO_AES library. */
static char m_plain_text[NRF_CRYPTO_EXAMPLE_AES_MAX_TEXT_SIZE] =
{
    "Example string demonstrating AES CBC_MAC. Text is 64 bytes long."
};

/* Exected calculated mac 16 bytes*/
static uint8_t expected_mac[AES_MAC_SIZE] =
{
    0x25, 0x42, 0x89, 0xD6, 0x10, 0x9C, 0xC4, 0x3B, 0x8C, 0x0A, 0xCF, 0x52, 0xEB, 0xB9, 0x33, 0x24
};

static void text_print(char const* p_label, char const * p_text, size_t len)
{
    NRF_LOG_RAW_INFO("----%s (length: %u) ----\r\n", p_label, len);
    NRF_LOG_FLUSH();
    for(size_t i = 0; i < len; i++)
    {
        NRF_LOG_RAW_INFO("%c", p_text[i]);
        NRF_LOG_FLUSH();
    }
    NRF_LOG_RAW_INFO("\r\n");
    NRF_LOG_RAW_INFO("---- %s end ----\r\n\r\n", p_label);
    NRF_LOG_FLUSH();
}

static void hex_text_print(char const* p_label, char const * p_text, size_t len)
{
    NRF_LOG_RAW_INFO("---- %s (length: %u) ----\r\n", p_label, len);
    NRF_LOG_FLUSH();

    // Handle partial line (left)
    for (size_t i = 0; i < len; i++)
    {
        if (((i & 0xF) == 0) && (i > 0))
        {
            NRF_LOG_RAW_INFO("\r\n");
            NRF_LOG_FLUSH();
        }

        NRF_LOG_RAW_INFO("%02x ", p_text[i]);
        NRF_LOG_FLUSH();
    }
    NRF_LOG_RAW_INFO("\r\n");
    NRF_LOG_RAW_INFO("---- %s end ----\r\n\r\n", p_label);
    NRF_LOG_FLUSH();
}


static void plain_text_print(void)
{
    size_t len = strlen(m_plain_text);

    text_print("Plain text", m_plain_text, len);
    hex_text_print("Plain text (hex)", m_plain_text, len);
}

static void mac_print(uint8_t const * p_buff, uint8_t mac_size)
{
    hex_text_print("Calculated MAC (hex)", (char const *)p_buff, mac_size);
}

static void crypt_cbc_mac(void)
{
    uint8_t     iv[NRF_CRYPTO_MBEDTLS_AES_IV_SIZE];
    uint8_t     mac[AES_MAC_SIZE];
    ret_code_t  ret_val;
    size_t      len_in;
    size_t      len_out;

    nrf_crypto_aes_context_t cbc_mac_128_ctx;

    memset(mac, 0, sizeof(mac));

    plain_text_print();

    /* Init cbc_mac context */
    ret_val = nrf_crypto_aes_init(&cbc_mac_128_ctx,
                                  &g_nrf_crypto_aes_cbc_mac_128_info,
                                  NRF_CRYPTO_MAC_CALCULATE);
    AES_ERROR_CHECK(ret_val);

    /* Set encryption key */
    ret_val = nrf_crypto_aes_key_set(&cbc_mac_128_ctx, m_key);
    AES_ERROR_CHECK(ret_val);

    /* Set IV */
    memset(iv, 0, sizeof(iv));

    ret_val = nrf_crypto_aes_iv_set(&cbc_mac_128_ctx, iv);
    AES_ERROR_CHECK(ret_val);

    len_in  = strlen(m_plain_text);
    len_out = sizeof(mac);

    /* Calculate MAC */
    ret_val = nrf_crypto_aes_finalize(&cbc_mac_128_ctx,
                                      (uint8_t *)m_plain_text,
                                      len_in,
                                      (uint8_t *)mac,
                                      &len_out);
    AES_ERROR_CHECK(ret_val);

    ret_val = nrf_crypto_aes_uninit(&cbc_mac_128_ctx);
    AES_ERROR_CHECK(ret_val);

    mac_print(mac, len_out);

    if (memcmp(mac, expected_mac, AES_MAC_SIZE) == 0)
    {
        NRF_LOG_RAW_INFO("AES CBC_MAC example executed successfully.\r\n");
    }
    else
    {
        NRF_LOG_RAW_INFO("AES CBC_MAC example failed!!!\r\n");
    }
}

int main(void)
{
    ret_code_t ret;

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_RAW_INFO("AES CBC_MAC example started.\r\n\r\n");
    NRF_LOG_FLUSH();

    ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);
    nrf_drv_clock_lfclk_request(NULL);

    ret = nrf_crypto_init();
    APP_ERROR_CHECK(ret);

    ret = nrf_mem_init();
    APP_ERROR_CHECK(ret);

    crypt_cbc_mac();
    while (true)
    {
        NRF_LOG_FLUSH();
        UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
    }
}

/** @} */
#endif // NRF_MODULE_ENABLED(NRF_CRYPTO)

