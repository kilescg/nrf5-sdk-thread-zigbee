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

#define ZB_TRACE_FILE_ID 15198

#include <stdlib.h>
#include "zboss_api.h"
#include "zb_nrf52_internal.h"

#include "sdk_config.h"
#if !defined SOFTDEVICE_PRESENT
#include <stdbool.h>
#include "nrf_ecb.h"
#else
#include "nrf_fstorage.h"
#endif

#include "nrf_log_ctrl.h"
#include "nrf_log.h"

#include "nrf_pwr_mgmt.h"

#ifndef ZIGBEE_VENDOR_OUI
#define ZIGBEE_VENDOR_OUI 16043574
#endif


#ifndef ZB_SLEEP_INVALID_VALUE
#define ZB_SLEEP_INVALID_VALUE (zb_uint32_t)(-1)
#endif

#if defined(NRF_LOG_BACKEND_UART_ENABLED) && NRF_LOG_BACKEND_UART_ENABLED
#include "nrf_log_backend_uart.h"
#include "nrf_drv_uart.h"
//TODO: get rid of extern by https://projecttools.nordicsemi.no/jira/browse/KRKNWK-2060
extern nrf_drv_uart_t m_uart;
#endif

static void zb_osif_rng_init(void);
static zb_uint32_t zb_osif_read_rndval(void);
static void zb_osif_aes_init(void);

#if defined SOFTDEVICE_PRESENT
/**
 * @brief Function used to inform radio driver about Softdevice's SoC events.
 *        Copied from nRF radio driver header files to avoid additional external dependencies inside SDK examples.
 *
 */
extern void nrf_raal_softdevice_soc_evt_handler(uint32_t evt_id);

static void zb_zboss_radio_driver_callback(uint32_t sys_evt)
{
  nrf_raal_softdevice_soc_evt_handler(sys_evt);
}

static void soc_evt_handler(uint32_t sys_evt, void * p_context)
{
  UNUSED_PARAMETER(p_context);

  switch (sys_evt)
  {
    case NRF_EVT_FLASH_OPERATION_SUCCESS:
    case NRF_EVT_FLASH_OPERATION_ERROR:
#ifdef NRF52840_XXAA
    case NRF_EVT_POWER_USB_POWER_READY:
    case NRF_EVT_POWER_USB_DETECTED:
    case NRF_EVT_POWER_USB_REMOVED:
#endif
      break;

    case NRF_EVT_HFCLKSTARTED:
    case NRF_EVT_RADIO_BLOCKED:
    case NRF_EVT_RADIO_CANCELED:
    case NRF_EVT_RADIO_SIGNAL_CALLBACK_INVALID_RETURN:
    case NRF_EVT_RADIO_SESSION_IDLE:
    case NRF_EVT_RADIO_SESSION_CLOSED:
      /* nRF Radio Driver softdevice event handler. */
      zb_zboss_radio_driver_callback(sys_evt);
      break;

    default:
      NRF_LOG_WARNING("Unexpected SOC event: 0x%x", sys_evt);
      break;
  }
}
#endif

#ifndef CONFIG_ZB_TEST_MODE_MAC
/**
 * WORKAROUND for KRKNWK-12678 / ZBS-692:
 * There is a bug in ZBOSS, leading to unbalanced address locks once a device looses
 * its connectivity with the network.
 * It is better to skip the address lock assertion and continue operation than halt or reset
 * the device.
 * In order to implement a selective assertion masking, the additional error handler has to be registered.
 * Its prototype as well as registering function are part of private ZBOSS API (see zb_sdo.h).
 * They are copied here just to implement the workaround.
 */
#define ZB_ADDRESS_FILE_ID                112
#define ZB_ADDRESS_UNLOCK_ASSERT_LINE_NUM 788

typedef void (*zb_assert_indication_cb_t)(zb_uint16_t file_id, zb_int_t line_number);

/**
  Register a callback which should be called when application falls into assert
 */
void zb_zdo_register_assert_indication_cb(zb_assert_indication_cb_t assert_cb);

static zb_bool_t m_skip_assertion = ZB_FALSE;

void assert_indication_cb(zb_uint16_t file_id, zb_int_t line_number)
{
  NRF_LOG_ERROR("ZBOSS assertion in file: %d, line: %d", file_id, line_number);
  zb_osif_serial_flush();

  if ((file_id == ZB_ADDRESS_FILE_ID) && (line_number == ZB_ADDRESS_UNLOCK_ASSERT_LINE_NUM) && (!ZB_JOINED()))
  {
    m_skip_assertion = ZB_TRUE;
  }
  else
  {
    m_skip_assertion = ZB_FALSE;
  }
}
#endif /* !defined(CONFIG_ZB_TEST_MODE_MAC) */

/**
   SoC general initialization
*/
void zb_osif_init(void)
{
  static zb_bool_t m_initialized = ZB_FALSE;

  if (m_initialized)
  {
    return;
  }

  m_initialized = ZB_TRUE;

#ifndef CONFIG_ZB_TEST_MODE_MAC
  zb_zdo_register_assert_indication_cb(assert_indication_cb);
#endif
  /* Initialise system timer */
  zb_osif_timer_init();
#if defined ZB_TRACE_OVER_USART && defined ZB_TRACE_LEVEL
  /* Initialise serial trace */
  zb_osif_serial_init();
#endif
  /* Initialise random generator */
  zb_osif_rng_init();

  /* Initialise AES ECB */
  zb_osif_aes_init();

#ifdef SOFTDEVICE_PRESENT
#ifdef ZB_ENABLE_SOFTDEVICE_FROM_ZBOSS
  /* Enable softdevice */
  {
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();

    APP_ERROR_CHECK(err_code);
  }
#endif
  NRF_SDH_SOC_OBSERVER(m_soc_observer, NRF_SDH_SOC_STACK_OBSERVER_PRIO, soc_evt_handler, NULL);
#endif

#ifdef ZB_USE_SLEEP
  /* Enable SoC sleep support */
  {
    ret_code_t ret = nrf_pwr_mgmt_init();
    ASSERT(ret == NRF_SUCCESS);
  }
#endif /*ZB_USE_SLEEP*/
}

void zb_osif_abort(void)
{
  NRF_LOG_ERROR("ZBOSS assertion occurred");
  zb_osif_serial_flush();

#ifndef CONFIG_ZB_TEST_MODE_MAC
  if (m_skip_assertion)
  {
    NRF_LOG_ERROR("KRKNWK-12678: Skip assertion handler.");
    return;
  }
#endif /* !defined(CONFIG_ZB_TEST_MODE_MAC) */

#if (defined CONFIG_ZBOSS_RESET_ON_ASSERT) && (CONFIG_ZBOSS_RESET_ON_ASSERT)
  zb_reset(0);
#endif

#if (defined CONFIG_ZBOSS_HALT_ON_ASSERT) && (CONFIG_ZBOSS_HALT_ON_ASSERT)
  while(1)
  {
#if defined ZB_NRF_TRACE && (defined ZB_TRACE_LEVEL || defined ZB_TRAFFIC_DUMP_ON)
    /* Flush all remaining logs, including reason of abort. */
    zb_osif_serial_flush();
#endif /* ZB_NRF_TRACE */
  }
#endif
}

void zb_reset(zb_uint8_t param)
{
  ZVUNUSED(param);
  
#ifdef ZB_NRF_INTERNAL
  NRF_LOG_ERROR("Reset MCU from ZBOSS");
  zb_osif_serial_flush();

  NVIC_SystemReset();
#endif
}

void zb_osif_enable_all_inter(void)
{
#ifdef SOFTDEVICE_PRESENT
  app_util_critical_region_exit(0);
#else
  __enable_irq();
#endif
}

void zb_osif_disable_all_inter(void)
{
#ifdef SOFTDEVICE_PRESENT
  uint8_t __CR_NESTED = 0;

  app_util_critical_region_enter(&__CR_NESTED);
#else
  __disable_irq();
#endif
}

zb_uint32_t zb_random_seed(void)
{
  return zb_osif_read_rndval();
}

/* Timer */

zb_uint32_t zb_get_utc_time(void)
{
  return ZB_TIME_BEACON_INTERVAL_TO_MSEC(ZB_TIMER_GET()) / 1000;
}

void zb_osif_busy_loop_delay(zb_uint32_t count)
{
  ZVUNUSED(count);
}


static void zb_osif_rng_init(void)
{
  ret_code_t err_code;
  err_code = nrf_drv_rng_init(NULL);
  if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_MODULE_ALREADY_INITIALIZED))
  {
    NRF_ERR_CHECK(err_code);
  }
  srand(zb_random_seed());
}


static zb_uint32_t zb_osif_read_rndval()
{
  zb_uint32_t rnd_val  = 0;
  zb_uint32_t err_code = 0;
  zb_uint8_t  length   = 0;

  /*wait complete randomization opration*/
  while (length < sizeof(rnd_val))
  {
    nrf_drv_rng_bytes_available(&length);
  }

  /*read random value*/
  err_code = nrf_drv_rng_rand((uint8_t *)&rnd_val, sizeof(rnd_val));
  if (err_code != NRF_SUCCESS)
    return 0;

  return rnd_val;
}

static void zb_osif_aes_init(void)
{
#if !defined SOFTDEVICE_PRESENT
  ret_code_t err_code;
  err_code = nrf_ecb_init();
  NRF_ERR_CHECK_BOOL(err_code);
#endif  
}

void zb_osif_aes128_hw_encrypt(zb_uint8_t *key, zb_uint8_t *msg, zb_uint8_t *c)
{
#if !defined SOFTDEVICE_PRESENT
  ret_code_t err_code;
#endif
  ZB_ASSERT(key);
  ZB_ASSERT(msg);
  ZB_ASSERT(c);
  if (!(c && msg && key))
  {
    return;
  }
#if !defined SOFTDEVICE_PRESENT  
  nrf_ecb_set_key(key);
  err_code = nrf_ecb_crypt(c, msg);
  NRF_ERR_CHECK_BOOL(err_code);
#else
  {
    nrf_ecb_hal_data_t ecb_data;

    ZB_MEMCPY(ecb_data.key, key, SOC_ECB_KEY_LENGTH );
    ZB_MEMCPY(ecb_data.cleartext, msg, SOC_ECB_CLEARTEXT_LENGTH );
    if (sd_ecb_block_encrypt(&ecb_data)!= NRF_SUCCESS)
    {
      ZB_ASSERT(0);
    }
    ZB_MEMCPY(c, ecb_data.ciphertext, SOC_ECB_CIPHERTEXT_LENGTH );
  }
#endif
}


zb_bool_t zb_osif_is_inside_isr(void)
{
/*
  return (zb_bool_t)(!!(SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk));
*/
  return (zb_bool_t)(__get_IPSR() != 0);
}

/**@brief Read IEEE long address from FICR registers. */
void zb_osif_get_ieee_eui64(zb_ieee_addr_t ieee_eui64)
{
  uint64_t factoryAddress;

  // Read random address from FICR.
  factoryAddress  = (uint64_t)NRF_FICR->DEVICEID[0] << 32;
  factoryAddress |= NRF_FICR->DEVICEID[1];

  // Set constant manufacturer ID to use MAC compression mechanisms.
  factoryAddress &= 0x000000FFFFFFFFFFLL;
  factoryAddress |= (uint64_t)(ZIGBEE_VENDOR_OUI) << 40;

  memcpy(ieee_eui64, &factoryAddress, sizeof(factoryAddress));
}

/**@brief Function which waits for event -- essential implementation of sleep on NRF52 */
void zb_osif_wait_for_event(void)
{
  /* In case of multiprotocol examples, the sd_app_evt_wait() function is called.
   * From functional point of view, it behaves like a single call of __WFE() instruction.
   * That means any interrupt that occurred between zb_osif_wait_for_event() calls will wake up the device.
   */
  nrf_pwr_mgmt_run();
}

/*lint -save -e(14) Weak linkage */
/**@brief Function which tries to sleep down the MCU
 *
 * Function is defined as weak; to be redefined if someone wants to implement their own
 * going-to-sleep policy.
 */
__WEAK void zb_osif_go_idle(void)
{
  zb_osif_wait_for_event();
}

ZB_WEAK_PRE void ZB_WEAK app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
  ZVUNUSED(id);
  ZVUNUSED(pc);
  ZVUNUSED(info);
  TRACE_MSG(TRACE_APS1, "app_error_fault_handler", (FMT__0));
  ZB_ASSERT(0);
}

/**@brief Function which disables all Zigbee stack-related peripherals
 *
 * Function is defined as weak; to be redefined if someone wants to implement their own
 * going-to-deep-sleep policy/share resources between Zigbee stack and other components.
 */
__WEAK void zb_nrf52_periph_disable(void)
{
  /* Previously deinitialise Random generator */
  nrf_drv_rng_uninit();

#if defined(ZB_NRF_TRACE)
#if defined(NRF_LOG_ENABLED) && (NRF_LOG_ENABLED == 1)
#if defined(NRF_LOG_BACKEND_UART_ENABLED) && NRF_LOG_BACKEND_UART_ENABLED
  /* Deinitialise the UART logging */
  nrf_drv_uart_uninit(&m_uart);
#endif /* #if defined(NRF_LOG_BACKEND_UART_ENABLED) && NRF_LOG_BACKEND_UART_ENABLED */
#endif
#endif
}

/**@brief Function which enables back all Zigbee stack-related peripherals
 *
 * Function is defined as weak; to be redefined if someone wants to implement their own
 * going-to-deep-sleep policy/share resources between Zigbee stack and other components.
 */
__WEAK void zb_nrf52_periph_enable(void)
{
  ret_code_t err_code;

  /* Restore the Random generator */
  err_code = nrf_drv_rng_init(NULL);
  NRF_ERR_CHECK(err_code);

#if defined(ZB_NRF_TRACE)
#if defined(NRF_LOG_ENABLED) && (NRF_LOG_ENABLED == 1)
#if defined(NRF_LOG_BACKEND_UART_ENABLED) && NRF_LOG_BACKEND_UART_ENABLED
  /* Restore UART logging */
  nrf_log_backend_uart_init();
#endif /* #if defined(NRF_LOG_BACKEND_UART_ENABLED) && NRF_LOG_BACKEND_UART_ENABLED */
#endif
#endif /* defined ZB_NRF_TRACE */
}

/**@brief Function which tries to put the device into deep sleep mode, caused by an empty Zigbee stack scheduler queue.
 *
 * Function is defined as weak; to be redefined if someone wants to implement their own
 * going-to-deep-sleep policy.
 */
__WEAK zb_uint32_t zb_osif_sleep(zb_uint32_t sleep_tmo)
{
  zb_uint32_t time_slept_ms = 0;

  if (!sleep_tmo)
  {
    return sleep_tmo;
  }

  /* Disable Zigbee stack-related peripherals to save energy. */
  zb_nrf52_periph_disable();

  /* Schedule an RTC timer interrupt to sleep for no longer than sleep_tmo. */
  zb_nrf52_sched_sleep(sleep_tmo);

  /* Wait for an event. */
  zb_osif_wait_for_event();

  /* Get RTC timer value to obtain sleep time. */
  time_slept_ms = zb_nrf52_get_time_slept();

  /* Enable Zigbee stack-related peripherals. */
  zb_nrf52_periph_enable();

  return time_slept_ms;
}

/**@brief Function which is called after zb_osif_sleep
 *        finished and ZBOSS timer is reenabled.
 *
 * Function is defined as weak; to be redefined if someone
 * wants to implement their own going-to-deep-sleep policy/share resources
 * between Zigbee stack and other components.
 */
__WEAK void zb_osif_wake_up(void)
{
#if ZB_TRACE_LEVEL
	ZB_SET_TRACE_ON();
#endif /* ZB_TRACE_LEVEL */
	/* Restore trace interrupts. TODO: Restore something else if needed */
}

/*lint -restore */
