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

#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/events/default/default-events.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
		
namespace _Esp32 {

static const OSystem::GraphicsMode _supportedGraphicsModes[] = {
	{ "default", "Default",	0},
	{ 0, 0, 0 }
};


const OSystem::GraphicsMode* OSystem_Esp32::getSupportedGraphicsModes() const {
	return _supportedGraphicsModes;
}

int OSystem_Esp32::getDefaultGraphicsMode() const {return 0;}

bool OSystem_Esp32::setGraphicsMode(int mode) {return true;}

int OSystem_Esp32::getGraphicsMode() const {return 0;}

void OSystem_Esp32::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	_gameWidth = width;
	_gameHeight = height;
}

void OSystem_Esp32::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	//printf("OSystem_Esp32::copyRectToScreen():%d,%d,%d,%d,%d\n",pitch,x,y,w,h);
	byte *dst = _gamePixels + y * _gameWidth + x;
	if (_gameWidth == pitch && pitch == w) {
		memcpy(dst, buf, h * w);
	} else {
		const byte *src = (const byte *)buf;
		do {
			memcpy(dst, src, w);
			src += pitch;
			dst += _gameWidth;
		} while (--h);
	}
//		setScreenDirty();
	
	
	
}

Graphics::Surface *OSystem_Esp32::lockScreen() {
	printf("*OSystem_Esp32::lockScreen()\n");
	
	_surface.init(_gameWidth, _gameHeight,
	#ifdef USE_RGB_COLOR
	_gameWidth * _pfGame.bytesPerPixel, _gamePixels, _pfGame
	#else
	_gameWidth, _gamePixels, Graphics::PixelFormat::createFormatCLUT8()
	#endif
	);
	return &_surface;
}


void OSystem_Esp32::unlockScreen() {
//		setScreenDirty();
	printf("OSystem_Esp32::unlockScreen()\n");
}
void OSystem_Esp32::updateScreen() {
//if(isScreenDirty())
  display_flush();
}
void OSystem_Esp32::setShakePos(int) {}
void OSystem_Esp32::showOverlay() {}
void OSystem_Esp32::hideOverlay() {}
Graphics::PixelFormat OSystem_Esp32::getOverlayFormat() const {return _pfGame;}
void OSystem_Esp32::clearOverlay() {}
void OSystem_Esp32::grabOverlay(void*, int) {
	printf("OSystem_Esp32::grabOverlay()\n");
}
void OSystem_Esp32::copyRectToOverlay(const void*, int, int, int, int, int) {
	printf("OSystem_Esp32::copyRectToOverlay()\n");
}
bool OSystem_Esp32::showMouse(bool visible) {
	bool last = _cursorVisible;
	_cursorVisible = visible;
	return last;
	//return false;
}

void OSystem_Esp32::warpMouse(int, int) {}

void OSystem_Esp32::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	_cursorWidth = w;
	_cursorHeight = h;
	_cursorHotspotX = hotspotX;
	_cursorHotspotY = hotspotY;
	_cursorKeyColor = keycolor;
	_mouseBuf = buf;

//	flushCursor();
//	warpMouse(_cursorX, _cursorY);
}

void OSystem_Esp32::setCursorPalette(const byte *colors, uint start, uint num) {
	byte r,g,b;
	for(int i = 0; i<num;i++){
		// Special conversion for RGB565
		r= colors[i*3];
		g= colors[i*3 + 1];
		b= colors[i*3 + 2];
		_cursorPalette[(start+i)*2 + 1] = ((b & 0xf8)>>3) | ((g & 0xfc) <<3);
		_cursorPalette[(start+i)*2] = (r&0xf8) | (g>>5);
	}
	//flushCursor();
}

void OSystem_Esp32::setPalette(const byte *colors, uint start, uint num){
	byte r,g,b;
	for(int i = 0; i<num;i++){
		// Special conversion for RGB565
		r= colors[i*3];
		g= colors[i*3 + 1];
		b= colors[i*3 + 2];
		_palette[(start+i)*2 + 1] = ((b & 0xf8)>>3) | ((g & 0xfc) <<3);
		_palette[(start+i)*2] = (r&0xf8) | (g>>5);
	}

}
//void OSystem_Esp32::grabPalette(byte *colors, uint start, uint num)  {}

bool OSystem_Esp32::hasFeature(OSystem::Feature f) {
	return (f == OSystem::kFeatureCursorPalette ||
	        f == OSystem::kFeatureOverlaySupportsAlpha);
}

void OSystem_Esp32::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureCursorPalette:
		_cursorPaletteEnabled = enable;
		flushCursor();
		break;
	default:
		break;
	}
}

bool OSystem_Esp32::getFeatureState(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureCursorPalette:
		return _cursorPaletteEnabled;
	default:
		return false;
	}
}

byte* OSystem_Esp32::getScreenBuffer() { return _gamePixels; }
int16 OSystem_Esp32::getHeight(){ return _gameHeight; }
int16 OSystem_Esp32::getWidth(){ return _gameWidth; }

void OSystem_Esp32::initGraphics(){
	_gameWidth = 320;
	_gameHeight = 200;
	_gamePixels = (byte *) malloc(_screen_width * _screen_height);
	for (uint32_t i = 0; i < _screen_width * _screen_height; i++)
	  _gamePixels[i] = 0;
}

void OSystem_Esp32::destroyGraphics()
{
  free(_gamePixels);
}

}  // namespace ESP32
