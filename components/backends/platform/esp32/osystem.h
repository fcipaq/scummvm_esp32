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

#ifndef PLATFORM_Esp32_H
#define PLATFORM_Esp32_H

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "backends/mutex/mutex.h"
#include "backends/base-backend.h"
#include "graphics/palette.h"
#include "base/main.h"
#include "audio/mixer_intern.h"
#include "audio/mixer.h"
#include "backends/graphics/graphics.h"
#include "components/graphics/surface.h"
#include "common/rect.h"
#include "common/queue.h"
#include "common/events.h"

#include "backends/plugins/posix/posix-provider.h"
#include "backends/fs/posix/posix-fs-factory.h"

#include <time.h>
#include <sys/time.h>
#include "esp_system.h"

#define TICKS_PER_MSEC 268123

#define NUM_BTNS (3)

namespace _Esp32 {

enum {
	GFX_LINEAR = 0,
	GFX_NEAREST = 1
};

enum InputMode {
	MODE_HOVER,
	MODE_DRAG,
};

enum {
    KEY_UP = 0,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_LEFT,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_MAX
};

struct esp_gamepad_state_t {
    uint8_t value[KEY_MAX];
};

static const OSystem::GraphicsMode s_graphicsModes[] = {
	{"default", "Default Test", GFX_LINEAR},
	{ 0, 0, 0 }
};

// todo: move this out of here and implement the regular GUI
/// Font data stored PER GLYPH
typedef struct {
	uint16_t bitmapOffset;     ///< Pointer into GFXfont->bitmap
	uint8_t  width;            ///< Bitmap dimensions in pixels
        uint8_t  height;           ///< Bitmap dimensions in pixels
	uint8_t  xAdvance;         ///< Distance to advance cursor (x axis)
	int8_t   xOffset;          ///< X dist from cursor pos to UL corner
        int8_t   yOffset;          ///< Y dist from cursor pos to UL corner
} GFXglyph_t;

/// Data stored for FONT AS A WHOLE
typedef struct { 
	uint8_t  *bitmap;      ///< Glyph bitmaps, concatenated
	GFXglyph_t *glyph;       ///< Glyph array
	uint8_t   first;       ///< ASCII extents (first char)
        uint8_t   last;        ///< ASCII extents (last char)
	uint8_t   yAdvance;    ///< Newline distance (y axis)
} GFXfont_t;

class OSystem_Esp32 : public EventsBaseBackend, public PaletteManager {
public:

	OSystem_Esp32();
	virtual ~OSystem_Esp32();

	void display_send_buf_raw(void* buffer, int width, int height, uint16_t* palette);

	volatile bool exiting;
	volatile bool sleeping;

        bool isInputInit();

	virtual void initBackend();
	
	virtual bool hasFeature(OSystem::Feature f);
	virtual void setFeatureState(OSystem::Feature f, bool enable);
	virtual bool getFeatureState(OSystem::Feature f);

	virtual bool pollEvent(Common::Event &event);

	virtual uint32 getMillis(bool skipRecord = false);
	virtual void delayMillis(uint msecs);
	virtual void getTimeAndDate(TimeDate &t) const;

	virtual MutexRef createMutex();
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual void logMessage(LogMessageType::Type type, const char *message);
        // audio
	virtual Audio::Mixer *getMixer();
	virtual PaletteManager *getPaletteManager() { return this; }
	virtual Common::String getSystemLanguage() const;

	//virtual void fatalError();

	virtual void quit();

	// HAL
	esp_gamepad_state_t hw_input_read_raw();
	void get_gamepad_state(esp_gamepad_state_t* out_state);
	

	//virtual Common::String getDefaultConfigFileName();

	// Graphics
	
	int getDefaultGraphicsMode() const;
	bool setGraphicsMode(int mode);
	void resetGraphicsScale();
	int getGraphicsMode() const;
	inline Graphics::PixelFormat getScreenFormat() const { return _pfGame; }
	//virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	Common::List<Graphics::PixelFormat> getSupportedFormats() const  {
		Common::List<Graphics::PixelFormat> list;
		list.push_back(Graphics::PixelFormat::createFormatCLUT8());
		return list;
	}
	void initSize(uint width, uint height,
	              const Graphics::PixelFormat *format = NULL);

	void setPalette(const byte *colors, uint start, uint num);
	void grabPalette(byte *colors, uint start, uint num) const override {}
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w,
	                      int h);
	Graphics::Surface *lockScreen();
	void unlockScreen();
	void updateScreen();
	void setShakePos(int shakeOffset);
	void setFocusRectangle(const Common::Rect &rect);
	void clearFocusRectangle();
	void showOverlay();
	void hideOverlay();
	Graphics::PixelFormat getOverlayFormat() const;
	void clearOverlay();
	void grabOverlay(void *buf, int pitch);
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w,
	                       int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	byte* getScreenBuffer();
	int16 getHeight();
	int16 getWidth();

	bool showMouse(bool visible);
	void warpMouse(int x, int y);
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX,
	                    int hotspotY, uint32 keycolor, bool dontScale = false,
	                    const Graphics::PixelFormat *format = NULL);
	void setCursorPalette(const byte *colors, uint start, uint num);

	// UIs
	void draw_string(uint16_t x, uint16_t y, const char* text, uint16_t color);
	
	// Events
	uint16_t getCursorPosX();
	uint16_t getCursorPosY();
	void setCursorPosX(uint16_t posx);
	void setCursorPosY(uint16_t posy);
	void report_keystate_to_engine();
	
	// HAL
	
	//void setCursorPalette(const byte *colors, uint start, uint num);

	// Transform point from touchscreen coords into gamescreen coords
	//void transformPoint(touchPosition &point);

	//void setCursorDelta(float deltaX, float deltaY);

	//void updateFocus();
	void updateConfig();
        //void updateSize();

	void setSaveDir(char* dir);
	
private:

	uint16_t _screen_width, _screen_height;

	uint16 _gameWidth, _gameHeight;

	struct timeval _startTime;
	
	char* savedir;

	// Input
        volatile bool _isInputInit = false;

	// Audio
	bool hasAudio;

	// Graphics
	Graphics::PixelFormat _pfGame;
	
	Graphics::PixelFormat _pfGameTexture;
	Graphics::PixelFormat _pfCursor;
	byte _palette[3 * 256];
	byte _cursorPalette[3 * 256];
	byte *_mouseBuf = NULL;
	byte *_gamePixels;
	Graphics::Surface _surface;
	
	// HW
	int _brightness;
	
	/*
	Sprite _gameTopTexture;
	Sprite _gameBottomTexture;
	Sprite _overlay;
	*/

	int _screenShakeOffset;
	bool _overlayVisible;

	Common::Queue<Common::Event> _eventQueue;

	// Cursor
	uint _cursorWidth = 16,	_cursorHeight = 16;
	int _cursorHotspotX = 8, _cursorHotspotY = 7;
	int _cursorKeyColor = 0;
	bool _cursorVisible = false;
	bool _cursorPaletteEnabled = false;
	
private:
	void initGraphics();
	void destroyGraphics();

	void initAudio(uint32_t audio_sample_rate);
	void destroyAudio();

	void initEvents();
	void destroyEvents();
	void pushEventQueue(Common::Queue<Common::Event> *queue, Common::Event &event);

	bool initSDcard();
	void destroySDcard();

	bool initDisplay();
	void destroyDisplay();
	void display_flush();
	void lcd_set_addr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
        void backlight_set(int value);
	int  is_backlight_initialized();

	void flushGameScreen();
	void flushCursor();
	void initInput();

	void draw_text(uint16_t x, uint16_t y, const char* message, uint16_t count, uint16_t color);
	void set_font(GFXfont_t* font);
	void writePixel(int x1,int y1, uint16_t color);
	void drawGlyph(int16_t x, int16_t y, GFXglyph_t *glyph, uint8_t  *bitmap, uint16_t color);
	uint16_t FntLineHeight();
	uint16_t FntLineWidth(const char* message, uint16_t count);

};

} // namespace _Esp32

#endif
