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

#pragma GCC optimize ("O3")

#include "osystem.h"

#include "esp_system.h"
#include "esp_event.h"
#include <string.h>

namespace _Esp32 {

#include "fonts/Font0_11px.h"
#include "fonts/Font1_11px.h"
#include "fonts/Font2_14px.h"
#include "fonts/Font7_15px.h"

GFXfont_t* gfxFont = &Font1_11px;

void OSystem_Esp32::draw_string(uint16_t x, uint16_t y, const char* text, uint16_t color){
	uint16_t background = 0xffff;
	
	uint16_t width = FntLineWidth(text, 256);
	uint16_t height = FntLineHeight();

	draw_text(x, y, text, 256, color);
}

void OSystem_Esp32::set_font(GFXfont_t* font){
	gfxFont = font;
}

void OSystem_Esp32::draw_text(uint16_t x, uint16_t y, const char* message, uint16_t count, uint16_t color){

	uint16_t i = 0;
	uint8_t first = gfxFont->first;
	
	while(message[i]!=0 && i<count){
		uint8_t c = message[i];
		GFXglyph_t *glyph = &(gfxFont->glyph[c - first]);
		drawGlyph(x, y, glyph, gfxFont->bitmap, color);
		x += glyph->xAdvance;
		//y += gfxFont->yAdvance;
		i++;
	}
}

void OSystem_Esp32::writePixel(int x1,int y1, uint16_t color){
	if((x1>=0) && (y1>=0) && (x1<_screen_width) && (y1<_screen_height)){
		_gamePixels[x1+((y1)*_screen_width)]=color;
	}
}

void OSystem_Esp32::drawGlyph(int16_t x, int16_t y, GFXglyph_t *glyph, uint8_t  *bitmap, uint16_t color){
	
	uint16_t bo = glyph->bitmapOffset;
	uint8_t  w  = glyph->width,
	h  = glyph->height;
	int8_t   xo = glyph->xOffset,
	yo = glyph->yOffset;

//	printf("(10) w,h,xo, yo: %d,%d,%d,%d\n", w,h,xo,yo);

	uint8_t  xx, yy, bits = 0, bit = 0;

	// Todo: Add character clipping here

	// NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
	// THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
	// has typically been used with the 'classic' font to overwrite old
	// screen contents with new data.  This ONLY works because the
	// characters are a uniform size; it's not a sensible thing to do with
	// proportionally-spaced fonts with glyphs of varying sizes (and that
	// may overlap).  To replace previously-drawn text when using a custom
	// font, use the getTextBounds() function to determine the smallest
	// rectangle encompassing a string, erase the area with fillRect(),
	// then draw new text.  This WILL infortunately 'blink' the text, but
	// is unavoidable.  Drawing 'background' pixels will NOT fix this,
	// only creates a new set of problems.  Have an idea to work around
	// this (a canvas object type for MCUs that can afford the RAM and
	// displays supporting setAddrWindow() and pushColors()), but haven't
	// implemented this yet.

	int x1,y1;

	for(yy=0; yy<h; yy++) {
		for(xx=0; xx<w; xx++) {
			if(!(bit++ & 7)) {
				bits = bitmap[bo++];
			}
			if(bits & 0x80) {
				x1 = x+xo+xx;
				y1 = y+yo+yy;
				writePixel(x1, y1, color);
			}
			bits <<= 1;
		}
	}
}

uint16_t OSystem_Esp32::FntLineHeight(){
	//GFXfont_t* gfxFont = &FreeSans9pt7b;
	return(gfxFont->yAdvance);
}

uint16_t OSystem_Esp32::FntLineWidth(const char* message, uint16_t count){
	uint16_t width = 0;
	uint16_t i = 0;
	//GFXfont_t* gfxFont = &FreeSans9pt7b;
	uint8_t first = gfxFont->first;
	while(message[i]!=0 && i < count){
		
		uint8_t c = message[i];
		GFXglyph_t *glyph = &(gfxFont->glyph[c - first]);
		width += glyph->xAdvance;
		i++;
	}
	return(width);
}

}  // namespace ESP32
