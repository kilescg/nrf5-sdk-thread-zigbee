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
 */
/* PURPOSE: osif layer for buttons & leds support for nrf52
*/

#include "app_pwm.h"
#include "boards.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"

#include "zboss_api.h"
#include "zb_led_button.h"

#ifdef ZB_USE_BUTTONS
#define BSP_INIT_FLAGS (BSP_INIT_LEDS | BSP_INIT_BUTTONS)
#else
#define BSP_INIT_FLAGS (BSP_INIT_LEDS)
#endif

/* Create the instance "PWM1" using TIMER2 */
APP_PWM_INSTANCE(PWM1, 2);


void pwm_ready_callback(uint32_t pwm_id)  // PWM callback function
{
  ZVUNUSED(pwm_id);
}

/* zb_osif_led_level_init
    param: led_no -  board led to control level (only one suported: ex: BSP_LED_2)
    returns true if PWM is successfully configured for a led, only one PWM led is supported
*/
zb_bool_t zb_osif_led_level_init(zb_uint8_t led_no)
{
  app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(5000L, bsp_board_led_idx_to_pin(led_no));

  if (app_pwm_init(&PWM1, &pwm1_cfg, pwm_ready_callback) == NRF_SUCCESS)
  {
    app_pwm_enable(&PWM1);
    return ZB_TRUE;
  }

  return ZB_FALSE;
}

/* requires pwm being configures first for this led */
void zb_osif_led_on_set_level(zb_uint8_t level)
{
  while (app_pwm_channel_duty_set(&PWM1, 0, level) == NRF_ERROR_BUSY)
  {
  };
}

#ifdef ZB_USE_BUTTONS
static void button_general_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  if(action == NRF_GPIOTE_POLARITY_TOGGLE)
  {
    if(zb_osif_button_state(bsp_board_pin_to_button_idx(pin)) == ZB_TRUE)
    {
      zb_button_on_cb(bsp_board_pin_to_button_idx(pin));
    }
    else
    {
      zb_button_off_cb(bsp_board_pin_to_button_idx(pin));
    }
  }
}

/* Configures buttons.
   To be called after the GPIOTE driver is initialized with nrf_drv_gpiote_init().
*/
static void button_init(void)
{
  nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
  zb_uint8_t buttons_in_use = (ZB_N_BUTTONS <= BUTTONS_NUMBER) ? ZB_N_BUTTONS: BUTTONS_NUMBER;

  in_config.pull = NRF_GPIO_PIN_PULLUP;

  /* Configure events for all the buttons available in the board */
  for (zb_uint8_t button_idx = 0; button_idx  < buttons_in_use; ++button_idx)
  {
    nrf_drv_gpiote_in_init(bsp_board_button_idx_to_pin(button_idx),
                           &in_config, button_general_cb); //lint !e534: Ignoring return value of function
    nrf_drv_gpiote_in_event_enable(bsp_board_button_idx_to_pin(button_idx), true);
  }
}
#endif

void zb_osif_led_button_init(void)
{
  if (nrf_drv_gpiote_is_init())
  {
    return;
  }

  bsp_board_init(BSP_INIT_FLAGS);

  nrf_drv_gpiote_init(); //lint !e534: Ignoring return value of function

#ifdef ZB_USE_BUTTONS
  button_init();
#endif
}

void zb_osif_led_on(zb_uint8_t led_no)
{
  bsp_board_led_on(led_no);
}

void zb_osif_led_off(zb_uint8_t led_no)
{
  bsp_board_led_off(led_no);
}

void zb_osif_led_toggle(zb_uint8_t led_no)
{
  bsp_board_led_invert(led_no);
}

zb_bool_t zb_osif_led_state(zb_uint8_t arg)
{
  zb_bool_t ret = ZB_FALSE;
  ret = (zb_bool_t)bsp_board_led_state_get(arg);
  return ret;
}

zb_bool_t zb_osif_button_state(zb_uint8_t arg)
{
  zb_bool_t ret = ZB_FALSE;
  ret = (zb_bool_t)bsp_board_button_state_get(arg);
  return ret;
}

void zb_osif_button_cb(zb_uint8_t arg)
{
  ZVUNUSED(arg);
}

zb_bool_t zb_setup_buttons_cb(zb_callback_t cb)
{
  ZVUNUSED(cb);

  return (zb_bool_t)(!nrf_drv_gpiote_is_init());
}
