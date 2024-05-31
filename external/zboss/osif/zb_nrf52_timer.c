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

#define ZB_TRACE_FILE_ID 52493

/* Fixes to make Lint passable - radio driver headers are not linted */
#ifndef RAAL_SOFTDEVICE
#define RAAL_SOFTDEVICE 0
#endif /* RAAL_SOFTDEVICE */

#include "zboss_api.h"

#ifdef ZB_STACK_REGRESSION_TESTING_API
#include "zb_common.h"
#endif  /* ZB_STACK_REGRESSION_TESTING_API */

#include "zb_nrf52_internal.h"
#include "timer_scheduler/nrf_802154_timer_sched.h"

#define MS_PER_S                  1000UL /* Miliseconds in one second. */
#define RADIO_DRIVER_RTC_OVRFLW_S 512UL  /* Time that has passed between overflow events. On full RTC speed, it occurs every 512 s. */
#define ZB_NRF52_MAX_SLEEP_PERIOD_MS  ((RADIO_DRIVER_RTC_OVRFLW_S - 1) * MS_PER_S) /* Zigbee max sleep period to prevent RTC timer overflow. */

static void rtc_event_handler(void * p_context);

static volatile zb_bool_t m_timer_is_init = ZB_FALSE;
static volatile zb_uint64_t m_timer_us = 0;
static volatile zb_uint32_t m_last_tstamp_us = 0;
static nrf_802154_timer_t m_timer;
static zb_uint32_t m_rtc_sleep_timer_val = 0;

/*
 * Get uptime value in us.
 */
static zb_uint64_t zb_now(void)
{
  zb_uint32_t now = nrf_802154_timer_sched_time_get();

  m_timer_us += (zb_uint64_t)(now - m_last_tstamp_us);
  m_last_tstamp_us = now;

  return m_timer_us;
}

/*
 * Get uptime value in us.
 */
zb_uint32_t zb_osif_timer_get(void)
{
  return (zb_uint32_t)zb_now();
}

void zb_osif_timer_init()
{
  m_last_tstamp_us = nrf_802154_timer_sched_time_get();
  m_timer_is_init = ZB_TRUE;
}

/*
 * Get ZBOSS time value in beacon intervals.
 * The single beacon interval duration in us is 15360us.
 */
zb_time_t zb_timer_get(void)
{
  return (zb_now()) / ((zb_uint64_t)(ZB_BEACON_INTERVAL_USEC));
}

void zb_osif_timer_stop()
{
}

void zb_osif_timer_start()
{
  ZB_OSIF_GLOBAL_LOCK();
  if (m_timer_is_init == ZB_FALSE)
  {
    zb_osif_timer_init();
  }
  ZB_OSIF_GLOBAL_UNLOCK();
}

zb_bool_t zb_osif_timer_is_on()
{
  return m_timer_is_init;
}

/**
 * Get current time, us.
 *
 * Use transceiver timer when possible.
 */
zb_time_t osif_transceiver_time_get(void)
{
  return zb_osif_timer_get();
}

void osif_sleep_using_transc_timer(zb_time_t timeout)
{
  zb_time_t tstart = osif_transceiver_time_get();
  zb_time_t tend   = tstart + timeout;

  if (tend < tstart)
  {
    while (tend < osif_transceiver_time_get())
    {
      zb_osif_busy_loop_delay(10);
    }
  }
  else
  {
    while (osif_transceiver_time_get() < tend)
    {
      zb_osif_busy_loop_delay(10);
    }
  }
}

static void rtc_event_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
}

void zb_nrf52_sched_sleep(zb_uint32_t sleep_tmo)
{
  m_rtc_sleep_timer_val = nrf_802154_timer_sched_time_get();

  if (!sleep_tmo)
  {
    return;
  }

  m_timer.callback  = rtc_event_handler;
  m_timer.p_context = NULL;
  m_timer.t0        = m_rtc_sleep_timer_val;
  /* Workaround for long zigbee sleep period.
   * Device wakes up on RTC event and scheduling sleep period
   * longer than RTC overflow (512 seconds) can result in very short sleep scheduled
   * whick wakes up CPU frequently and increases power consumptiom.
   */
  m_timer.dt        = 1000 * ((sleep_tmo > (ZB_NRF52_MAX_SLEEP_PERIOD_MS)) ? (ZB_NRF52_MAX_SLEEP_PERIOD_MS) : (sleep_tmo));

  nrf_802154_timer_sched_add(&m_timer, false);
  ZVUNUSED(zb_now());
}

zb_uint32_t zb_nrf52_get_time_slept(void)
{
  zb_uint32_t rtc_wait_val = 0;

  /* Calculate real completed time (us) */
  rtc_wait_val = nrf_802154_timer_sched_time_get() - m_rtc_sleep_timer_val;
  ZVUNUSED(zb_now());

  return ((rtc_wait_val + 999) / 1000);
}
