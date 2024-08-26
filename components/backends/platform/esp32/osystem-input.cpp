/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "osystem.h"

#include "freertos/FreeRTOS.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"

#include <string.h>

#define KEY_PIN_X ADC_CHANNEL_6
#define KEY_PIN_Y ADC_CHANNEL_7
#define KEY_PIN_1 GPIO_NUM_5
#define KEY_PIN_2 GPIO_NUM_6
#define KEY_PIN_3 GPIO_NUM_4

namespace _Esp32 {

// thread safetly
volatile SemaphoreHandle_t _lock_gpio_read = xSemaphoreCreateBinary();

static adc_oneshot_unit_handle_t adc1_handle;

esp_gamepad_state_t OSystem_Esp32::hw_input_read_raw() {
  OSystem_Esp32* osys = (OSystem_Esp32*) g_system;
  
  esp_gamepad_state_t state = { 0 };

  if (!(osys->isInputInit()))
    return state;

  xSemaphoreTake(_lock_gpio_read, portMAX_DELAY);

  int32_t joyX;
  int32_t joyY;

  adc_oneshot_read(adc1_handle, KEY_PIN_X, &joyX);
  adc_oneshot_read(adc1_handle, KEY_PIN_Y, &joyY);

  if (joyX > (2000 + 500)) {
    state.value[KEY_LEFT] = 1;
    state.value[KEY_RIGHT] = 0;
  } else if (joyX < (2000 - 500)) {
    state.value[KEY_LEFT] = 0;
    state.value[KEY_RIGHT] = 1;
  } else {
    state.value[KEY_LEFT] = 0;
    state.value[KEY_RIGHT] = 0;
  }

  if (joyY > (1900 + 500)) {
    state.value[KEY_UP] = 0;
    state.value[KEY_DOWN] = 1;
  } else if (joyY < (1800 - 500)) {
    state.value[KEY_UP] = 1;
    state.value[KEY_DOWN] = 0;
  } else {
    state.value[KEY_UP] = 0;
    state.value[KEY_DOWN] = 0;
  }

  state.value[KEY_1] = gpio_get_level(KEY_PIN_1);
  state.value[KEY_2] = gpio_get_level(KEY_PIN_2);
  state.value[KEY_3] = gpio_get_level(KEY_PIN_3);
  
  xSemaphoreGive(_lock_gpio_read);

  return state;
}

bool OSystem_Esp32::isInputInit() {
  return _isInputInit;
}

void OSystem_Esp32::initInput() {
  assert(!_isInputInit);

  assert(_lock_gpio_read);
  xSemaphoreGive(_lock_gpio_read);

  gpio_set_direction(KEY_PIN_1, GPIO_MODE_INPUT);
  gpio_set_pull_mode(KEY_PIN_1, GPIO_PULLDOWN_ONLY);

  gpio_set_direction(KEY_PIN_2, GPIO_MODE_INPUT);
  gpio_set_pull_mode(KEY_PIN_2, GPIO_PULLDOWN_ONLY);

  gpio_set_direction(KEY_PIN_3, GPIO_MODE_INPUT);
  gpio_set_pull_mode(KEY_PIN_3, GPIO_PULLDOWN_ONLY);


  //-------------ADC1 Init---------------//
  adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

  //-------------ADC1 Config---------------//
  adc_oneshot_chan_cfg_t config = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, KEY_PIN_X, &config));
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, KEY_PIN_Y, &config));

  /*
    //-------------ADC1 Calibration Init---------------//
    adc_cali_handle_t adc1_cali_chan0_handle = NULL;
    adc_cali_handle_t adc1_cali_chan1_handle = NULL;
    bool do_calibration1_chan0 = example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN0, EXAMPLE_ADC_ATTEN, &adc1_cali_chan0_handle);
    bool do_calibration1_chan1 = example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN1, EXAMPLE_ADC_ATTEN, &adc1_cali_chan1_handle);
*/

  _isInputInit = true;

}

}  // namespace ESP32
