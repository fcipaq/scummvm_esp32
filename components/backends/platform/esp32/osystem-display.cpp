/*
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
#include "esp_system.h"
#include "driver/ledc.h"
#include "driver/rtc_io.h"

#include <string.h>

/* ------------------------ LCD pin assignment ------------------------*/
#define PIN_LCD_BL    GPIO_NUM_39

#define PIN_LCD_DC    GPIO_NUM_14   // DC control pin
#define PIN_LCD_WR    GPIO_NUM_15   // clock
#define PIN_LCD_RST   GPIO_NUM_16   // reset pin
#define PIN_LCD_TE    GPIO_NUM_13   // tearing pin

#if 0
// picoheld 2 MB PSRAM
#define PIN_LCD_D0    GPIO_NUM_17
#define PIN_LCD_D1    GPIO_NUM_18
#define PIN_LCD_D2    GPIO_NUM_33
#define PIN_LCD_D3    GPIO_NUM_34
#define PIN_LCD_D4    GPIO_NUM_35
#define PIN_LCD_D5    GPIO_NUM_36
#define PIN_LCD_D6    GPIO_NUM_37
#define PIN_LCD_D7    GPIO_NUM_38
#endif

#if 1
// picoheld 8 MB PSRAM
#define PIN_LCD_D0    GPIO_NUM_17
#define PIN_LCD_D1    GPIO_NUM_18
#define PIN_LCD_D2    GPIO_NUM_21
#define PIN_LCD_D3    GPIO_NUM_44
#define PIN_LCD_D4    GPIO_NUM_0
#define PIN_LCD_D5    GPIO_NUM_41
#define PIN_LCD_D6    GPIO_NUM_40
#define PIN_LCD_D7    GPIO_NUM_38
#endif

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          PIN_LCD_BL
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY          (100000)

/* ---------- LCD ------------*/
#define LCD_ENABLE_OUTPUT
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#define PHYS_SCREEN_WIDTH  (320)
#define PHYS_SCREEN_HEIGHT (240)

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_Parallel8 _bus_instance; // 8ビットパラレルバスのインスタンス (ESP32のみ)

public:
    // コンストラクタを作成し、ここで各種設定を行います。
    // クラス名を変更した場合はコンストラクタも同じ名前を指定してください。
    LGFX(void)
    {
        {                                      // バス制御の設定を行います。
            auto cfg = _bus_instance.config(); // バス設定用の構造体を取得します。

            // 16位设置
            cfg.port = 0;              // 使用するI2Sポートを選択 (0 or 1) (ESP32のI2S LCDモードを使用します)
            cfg.freq_write = 20000000; // 送信クロック (最大20MHz, 80MHzを整数で割った値に丸められます)
            cfg.pin_wr = PIN_LCD_WR;           // WR を接続しているピン番号
            cfg.pin_rd = -1;           // RD を接続しているピン番号
            cfg.pin_rs = PIN_LCD_DC;           // RS(D/C)を接続しているピン番号

            cfg.pin_d0 = PIN_LCD_D0;
            cfg.pin_d1 = PIN_LCD_D1;
            cfg.pin_d2 = PIN_LCD_D2;
            cfg.pin_d3 = PIN_LCD_D3;
            cfg.pin_d4 = PIN_LCD_D4;
            cfg.pin_d5 = PIN_LCD_D5;
            cfg.pin_d6 = PIN_LCD_D6;
            cfg.pin_d7 = PIN_LCD_D7;

            _bus_instance.config(cfg);              // 設定値をバスに反映します。
            _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
        }

        {                                        // 表示パネル制御の設定を行います。
            auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

            cfg.pin_cs = -1;   // CS要拉低
            cfg.pin_rst = PIN_LCD_RST;  // RST和开发板RST相连
            cfg.pin_busy = -1; // PIN_LCD_TE

            // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

            cfg.memory_width = PHYS_SCREEN_HEIGHT;   // ドライバICがサポートしている最大の幅
            cfg.memory_height = PHYS_SCREEN_WIDTH;  // ドライバICがサポートしている最大の高さ
            cfg.panel_width = PHYS_SCREEN_HEIGHT;    // 実際に表示可能な幅
            cfg.panel_height = PHYS_SCREEN_WIDTH;   // 実際に表示可能な高さ
            cfg.offset_x = 0;         // パネルのX方向オフセット量
            cfg.offset_y = 0;         // パネルのY方向オフセット量
            cfg.offset_rotation = 3;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
            cfg.dummy_read_pixel = -1; // ピクセル読出し前のダミーリードのビット数
            cfg.dummy_read_bits = -1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
            cfg.readable = false;      // データ読出しが可能な場合 trueに設定
            cfg.invert = true;       // パネルの明暗が反転してしまう場合 trueに設定
            cfg.rgb_order = true;    // パネルの赤と青が入れ替わってしまう場合 trueに設定
            cfg.dlen_16bit = false;    // データ長を16bit単位で送信するパネルの場合 trueに設定
            cfg.bus_shared = false;    // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance); // 使用するパネルをセットします。
    }
};

namespace _Esp32 {

static uint16_t _lcd_addr_x1_old = 0;
static uint16_t _lcd_addr_y1_old = 0;
static uint16_t _lcd_addr_x2_old = 0;
static uint16_t _lcd_addr_y2_old = 0;

static uint16_t* line_buffer = (uint16_t*) heap_caps_malloc(640, MALLOC_CAP_DMA);

static bool _isBackLightIntialized = false;

static LGFX lcd;
/* -------- LCD END -----------*/

void OSystem_Esp32::backlight_set(int value)
{
    if (!is_backlight_initialized())
      return;

    if (value < 0)
      value = 0;

    if (value > 100)
      value = 100;
      
    int duty = value * 255 / 100;

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));

    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

    // //set the configuration
    // ledc_channel_config_t ledc_channel;
    // memset(&ledc_channel, 0, sizeof(ledc_channel));
    //
    // //set LEDC channel 0
    // ledc_channel.channel = LEDC_CHANNEL_0;
    // //set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
    // ledc_channel.duty = duty;
    // //GPIO number
    // ledc_channel.gpio_num = LCD_PIN_NUM_BCKL;
    // //GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
    // ledc_channel.intr_type = LEDC_INTR_FADE_END;
    // //set LEDC mode, from ledc_mode_t
    // ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    // //set LEDC timer source, if different channel use one timer,
    // //the frequency and bit_num of these channels should be the same
    // ledc_channel.timer_sel = LEDC_TIMER_0;
    //
    //
    // ledc_channel_config(&ledc_channel);

    //ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty, 500);
    //ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
}

/*
static uint16_t Blend(uint16_t a, uint16_t b)
{
  // Big endian
  // rrrrrGGG gggbbbbb

  char r0 = (a >> 11) & 0x1f;
  char g0 = (a >> 5) & 0x3f;
  char b0 = (a) & 0x1f;

  char r1 = (b >> 11) & 0x1f;
  char g1 = (b >> 5) & 0x3f;
  char b1 = (b) & 0x1f;

  uint16_t rv = ((r1 - r0) >> 1) + r0;
  uint16_t gv = ((g1 - g0) >> 1) + g0;
  uint16_t bv = ((b1 - b0) >> 1) + b0;

  return (rv << 11) | (gv << 5) | (bv);
}
*/
void OSystem_Esp32::display_send_buf_raw(void* buffer, int width, int height, uint16_t* palette) {
        lcd_set_addr(0, 0, width, height);

	if (palette) {
          uint32_t bufferIndex = 0;
	  uint8_t* localbuf = (uint8_t*) buffer;
  	  for (int y = 0; y < height; y++) {
	    for (int x = 0; x < width; x++)
	      line_buffer[x] = palette[localbuf[bufferIndex++]];
	    lcd.waitDisplay();
	    lcd.pushPixels((uint16_t*) line_buffer, width);
	  }
	} else {
	    lcd.pushPixels((uint16_t*) buffer, width * height);
	}
}

void OSystem_Esp32::display_flush() {
	uint16_t* palette = (uint16_t*) _palette;

	uint16_t* cursorPalette;

	if (_cursorPaletteEnabled)
	  cursorPalette = (uint16_t*) _cursorPalette;
	else
	  cursorPalette = (uint16_t*) _palette;

	int cx = getCursorPosX() - _cursorHotspotX;
	int cy = getCursorPosY() - _cursorHotspotY;
	  lcd_set_addr(0, 0, _screen_width, _screen_height);
	 
	  uint32_t index = 0;
	  uint32_t bufferIndex = 0;
	  
	  uint16_t num_lines = _gameHeight;
  
	for (int y = 0; y < num_lines; y++) {
	  for (int x = 0; x < _screen_width; ++x) {

		 line_buffer[x] = palette[_gamePixels[bufferIndex++]];

		 if ((_mouseBuf) && (_cursorVisible))
		   if (_mouseBuf[x - cx + (y - cy) * _cursorWidth] != _cursorKeyColor) 
		     if ( ( x >= cx ) &&
		          ( x < (cx + _cursorWidth) ) &&
		          ( y >= cy ) &&
		          ( y < (cy + _cursorHeight) ) )
			line_buffer[x]= cursorPalette[_mouseBuf[x - cx + (y - cy) * _cursorWidth]];

	  }
#ifdef LCD_ENABLE_OUTPUT	  
	  lcd.waitDisplay();
	  lcd.pushPixels((uint16_t*) line_buffer, _screen_width);
	  if (y % 5 == 0)
	    lcd.pushPixels((uint16_t*) line_buffer, _screen_width);
#endif
	}
}

bool OSystem_Esp32::initDisplay()
{
#ifdef LCD_ENABLE_OUTPUT
 lcd.init();

  lcd.startWrite();

  lcd.setColorDepth(16);
#endif
  
  gpio_reset_pin((gpio_num_t) PIN_LCD_BL);

    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_USE_XTAL_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = PIN_LCD_BL,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    _isBackLightIntialized = true;    
    
    backlight_set(_brightness);

    _screen_width = PHYS_SCREEN_WIDTH;
    _screen_height = PHYS_SCREEN_HEIGHT;
	
  return true;
}

void OSystem_Esp32::lcd_set_addr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
 if ((x1 == _lcd_addr_x1_old) && (x2 == _lcd_addr_x2_old) && (y1 == _lcd_addr_y1_old) && (y2 == _lcd_addr_y2_old))
   return;
   
 _lcd_addr_x1_old = x1;
 _lcd_addr_y1_old = y1;
 _lcd_addr_x2_old = x2;
 _lcd_addr_y2_old = y2;

#ifdef LCD_ENABLE_OUTPUT 
 lcd.setAddrWindow(x1, y1, x2, y2);
#endif
}

void OSystem_Esp32::destroyDisplay()
{
}

int OSystem_Esp32::is_backlight_initialized()
{
    return _isBackLightIntialized;
}

} // namespace ESP32
