/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#include "Panel_LCD.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_ST7789  : public Panel_LCD
  {
    Panel_ST7789(void)
    {
      _cfg.panel_height = _cfg.memory_height = 320;

      _cfg.dummy_read_pixel = 16;
    }

  protected:

    static constexpr uint8_t CMD_RAMCTRL  = 0xB0;
    static constexpr uint8_t CMD_PORCTRL  = 0xB2;      // Porch control
    static constexpr uint8_t CMD_GCTRL    = 0xB7;      // Gate control
    static constexpr uint8_t CMD_VCOMS    = 0xBB;      // VCOMS setting
    static constexpr uint8_t CMD_LCMCTRL  = 0xC0;      // LCM control
    static constexpr uint8_t CMD_VDVVRHEN = 0xC2;      // VDV and VRH command enable
    static constexpr uint8_t CMD_VRHS     = 0xC3;      // VRH set
    static constexpr uint8_t CMD_VDVSET   = 0xC4;      // VDV setting
    static constexpr uint8_t CMD_FRCTR2   = 0xC6;      // FR Control 2
    static constexpr uint8_t CMD_PWCTRL1  = 0xD0;      // Power control 1
    static constexpr uint8_t CMD_PVGAMCTRL= 0xE0;      // Positive voltage gamma control
    static constexpr uint8_t CMD_NVGAMCTRL= 0xE1;      // Negative voltage gamma control

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
//        CMD_SLPOUT , CMD_INIT_DELAY, 120,
//        CMD_NORON  , CMD_INIT_DELAY, 0,
//        0xB6       , 2, 0x0A,0x82,

          0x3a, 1, 0x55,

          0x36, 1, 0,

          0xb0, 2, 0, 0xc0,

          0xb2, 5, 0x0c, 0x0c, 0, 0x33, 0x33,

          0xb7, 1, 0x35,
          0xbb, 1, 0x19,
          0xc0, 1, 0x2c,
          0xc2, 1, 0x01,
          0xc3, 1, 0x12,
          0xc4, 1, 0x20,

          0xC6, 1, 0x0F,
          0xD0  , 2, 0xA4, 0xA1,


          //--------------------------------ST7789V gamma setting---------------------------------------//
          0xe0,14, 0b11110000,0b00001001,0b00010011,0b00010010,
                             0b00010010,0b00101011,0b00111100,0b01000100,
                             0b01001011,0b00011011,0b00011000,0b00010111,
                             0b00011101,0b00100001,
          0xe1,14, 0b11110000,0b00001001,0b00010011,0b00001100,
                             0b00001101,0b00100111,0b00111011,0b01000100,
                             0b01001101,0b00001011,0b00010111,0b00010111,
                             0b00011101,0b00100001,
          0x21, 0,
          
          CMD_SLPOUT, 0+CMD_INIT_DELAY, 130,    // Exit sleep mode
          CMD_IDMOFF, 0,
          CMD_DISPON, 0,
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------
 }
}
