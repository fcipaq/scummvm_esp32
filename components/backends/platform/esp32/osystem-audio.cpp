/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Adapted to work with ESP32 PWM audio output - daniel.kammer@web.de
 */

#include "osystem.h"

#include "audio/mixer.h"
#include "backends/base-backend.h"
#include "audio/mixer_intern.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
//#include "esp_timer.h"
//#include "hal/timer_types.h"
#include "driver/gptimer.h"
#include "soc/ledc_struct.h"
#include "driver/ledc.h"

#define BUF_SIZE_SAMPLES 8192  // needs to be even
#define BUF_NUM_MAX      (2)
#define MAX_VOL          (0)    // 0: loudest, LEDC_DUTY_RES: mute

#define PIN_SND          GPIO_NUM_11
#define LEDC_TIMER       LEDC_TIMER_3
#define LEDC_MODE        LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL     LEDC_CHANNEL_7
#define LEDC_DUTY_RES    LEDC_TIMER_8_BIT

namespace _Esp32 {

bool _isAudioInit = false;

Audio::MixerImpl* _mixer = 0;

TaskHandle_t audioThreadFunc_handle = NULL;

gptimer_handle_t _periodic_timer = NULL;
    
volatile SemaphoreHandle_t _buffer_ready;
volatile SemaphoreHandle_t _pwm_go;

static uint16_t _audio_sample_rate = 0;

struct audio_buffer_t {
  volatile uint32_t nsamples;
  int16_t* pcmdata;
};

audio_buffer_t _audio_buf[BUF_NUM_MAX];

volatile int _cur_buf_write;
volatile int _cur_buf_read;
volatile uint32_t _play_index;
volatile int _pwm_stalled = 0;

inline void IRAM_ATTR set_pwm_duty_fast(uint16_t value) {
  // write new duty value
  LEDC.channel_group[LEDC_MODE].channel[LEDC_CHANNEL].duty.val = value;

  // apply new duty value
  LEDC.channel_group[LEDC_MODE].channel[LEDC_CHANNEL].conf0.sig_out_en = 1;
  LEDC.channel_group[LEDC_MODE].channel[LEDC_CHANNEL].conf0.low_speed_update = 1;  // para_up
  LEDC.channel_group[LEDC_MODE].channel[LEDC_CHANNEL].conf1.duty_start = 1;
}

static void IRAM_ATTR pwm_updater(void* arg) {
  if (_play_index == 0) {
    if (xSemaphoreTake(_pwm_go, 0) != pdTRUE) {
      if (!_pwm_stalled) {
        // stall the output if there are no buffers to play
        set_pwm_duty_fast(0);
	_pwm_stalled = 1;
      }
      return;
    }
  }

  set_pwm_duty_fast((_audio_buf[_cur_buf_read].pcmdata[_play_index] + 32768) >> (8 + MAX_VOL - 4));  // -4 to discard fractional part
    
  _play_index += 2;  // skip stereo samples

  if (_play_index >= _audio_buf[_cur_buf_read].nsamples * 2 ) {
    _cur_buf_read += 1;
    _cur_buf_read %= BUF_NUM_MAX;
    _play_index = 0;
    _pwm_stalled = 0;
    xSemaphoreGive(_buffer_ready);
  }
}

static void audioThreadFunc(void* arg) {
  // audio handling loop
  while (1) {
    while ( xSemaphoreTake(_buffer_ready, portMAX_DELAY) != pdTRUE)
      ;

    // CAUTION: mixCallback apparently double functions as some sort of pace maker
    // for sound sample generation.
    // So calling mixCallback too freuquently will make the sound run too fast.
    do {
      _audio_buf[_cur_buf_write].nsamples = _mixer->mixCallback((byte*)_audio_buf[_cur_buf_write].pcmdata, BUF_SIZE_SAMPLES);
      vTaskDelay(1 / portTICK_PERIOD_MS);
    } while (!(_audio_buf[_cur_buf_write].nsamples));

    _cur_buf_write += 1;
    _cur_buf_write %= BUF_NUM_MAX;
    
    xSemaphoreGive(_pwm_go);
    
  }

  for (int i = 0; i < BUF_NUM_MAX; i++)
    free(_audio_buf[i].pcmdata);
}

void OSystem_Esp32::initAudio(uint32_t audio_sample_rate) {
  assert(!_isAudioInit);
  
  _audio_sample_rate = audio_sample_rate;

  _mixer = new Audio::MixerImpl(g_system, _audio_sample_rate);

  assert(_mixer);
  
  // Setup PWM & PWM timer
  ledc_timer_config_t ledc_timer = {
    .speed_mode = LEDC_MODE,
    .duty_resolution = LEDC_DUTY_RES,
    .timer_num = LEDC_TIMER,
    .freq_hz = _audio_sample_rate * 4,  // relation between max frequency and duty_res: LEDC_USE_XTAL_CLK / (2 ^ max_bitnum) =  frequency_max
    .clk_cfg = LEDC_USE_XTAL_CLK
  };

  assert(ledc_timer_config(&ledc_timer) == ESP_OK);

  ledc_channel_config_t ledc_channel = {
    .gpio_num = PIN_SND,
    .speed_mode = LEDC_MODE,
    .channel = LEDC_CHANNEL,
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER,
    .duty = 0,
    .hpoint = 0
  };

  assert(ledc_channel_config(&ledc_channel) == ESP_OK);

  _pwm_go = xSemaphoreCreateCounting(BUF_NUM_MAX, 0);
  assert(_pwm_go);
  _buffer_ready = xSemaphoreCreateCounting(BUF_NUM_MAX, BUF_NUM_MAX);
  assert(_buffer_ready);

  for (int i = 0; i < BUF_NUM_MAX; i++) {
    _audio_buf[i].pcmdata = (int16_t*) heap_caps_malloc((BUF_SIZE_SAMPLES + 1) * 4, MALLOC_CAP_SPIRAM); //  MALLOC_CAP_DMA   
    assert(_audio_buf[i].pcmdata);
  }

  _cur_buf_write = 0;
  _cur_buf_read = 0;
  _play_index = 0;
  
  // configure callback timer 
  gptimer_config_t timer_config = {
      .clk_src = GPTIMER_CLK_SRC_DEFAULT,
      .direction = GPTIMER_COUNT_UP,
      .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
  };

  ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &_periodic_timer));

  gptimer_alarm_config_t alarm_config;
  alarm_config.reload_count = 0;
  alarm_config.alarm_count = 1000000 / _audio_sample_rate;
  alarm_config.flags.auto_reload_on_alarm = true;

  ESP_ERROR_CHECK(gptimer_set_alarm_action(_periodic_timer, &alarm_config));

  gptimer_event_callbacks_t cbs;
  cbs.on_alarm = pwm_updater;

  ESP_ERROR_CHECK(gptimer_register_event_callbacks(_periodic_timer, &cbs, NULL));
  ESP_ERROR_CHECK(gptimer_enable(_periodic_timer));
  ESP_ERROR_CHECK(gptimer_start(_periodic_timer));
  
  _isAudioInit = true;

  // start thread
  xTaskCreatePinnedToCore(&audioThreadFunc, "audioThreadFunc", 8192, NULL, 5, &audioThreadFunc_handle, 1);

  _mixer->setReady(true);
}

Audio::Mixer* OSystem_Esp32::getMixer() {
  assert(_isAudioInit);
  assert(_mixer);
  return _mixer;
}

void OSystem_Esp32::destroyAudio() {
  if (_isAudioInit) {
    // kill the PWM output
    gptimer_stop(_periodic_timer);  // unsure if necessary
    gptimer_disable(_periodic_timer);
    gptimer_del_timer(_periodic_timer);

    set_pwm_duty_fast(0);
    
    // stop audio handling loop
    vTaskDelete( audioThreadFunc_handle );
    vSemaphoreDelete(_pwm_go);
    vSemaphoreDelete(_buffer_ready);

    for (int i = 0; i < BUF_NUM_MAX; i++)
      free(_audio_buf[i].pcmdata);

    _mixer->setReady(false);
    delete _mixer;
    _mixer = NULL;

    _isAudioInit = false;
  }

}

} // namespace ESP32
