/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "queen/display.h"

#include "queen/input.h"
#include "queen/queen.h"
#include "queen/resource.h"

namespace Queen {


void TextRenderer::init() {
	// calculate font justification sizes
	uint16 i, y, x;

	for (i = 0; i < 256; ++i) {
		_charWidth[i] = 0;
		for (y = 0; y < 8; ++y) {
			uint8 c = _font[i * 8 + y];
			for (x = 0; x < 8; ++x) {
				if ((c & (0x80 >> x)) && (x > _charWidth[i])) {
					_charWidth[i] = x;
				}
			}
		}
		_charWidth[i] += 2;
	}
	_charWidth[(uint8)' '] = 4;
	--_charWidth[(uint8)'^'];
}


void TextRenderer::drawString(uint8 *dstBuf, uint16 dstPitch, uint16 x, uint16 y, uint8 color, const char *text, bool outlined) {
	const uint8 *str = (const uint8*)text;
	while (*str && x < dstPitch) {

		uint8 c = (_lang == FRENCH && *str == 0x96) ? 0xFB : *str;
		const uint8 *pchr = _font + c * 8;

		if (outlined) {
			drawChar(dstBuf, dstPitch, x - 1, y - 1, INK_OUTLINED_TEXT, pchr);
			drawChar(dstBuf, dstPitch, x    , y - 1, INK_OUTLINED_TEXT, pchr);
			drawChar(dstBuf, dstPitch, x + 1, y - 1, INK_OUTLINED_TEXT, pchr);
			drawChar(dstBuf, dstPitch, x + 1, y    , INK_OUTLINED_TEXT, pchr);
			drawChar(dstBuf, dstPitch, x + 1, y + 1, INK_OUTLINED_TEXT, pchr);
			drawChar(dstBuf, dstPitch, x    , y + 1, INK_OUTLINED_TEXT, pchr);
			drawChar(dstBuf, dstPitch, x - 1, y + 1, INK_OUTLINED_TEXT, pchr);
			drawChar(dstBuf, dstPitch, x - 1, y    , INK_OUTLINED_TEXT, pchr);
		}
		drawChar(dstBuf, dstPitch, x, y, color, pchr);

		x += _charWidth[ c ];
		++str;
	}
}


void TextRenderer::drawChar(uint8 *dstBuf, uint16 dstPitch, uint16 x, uint16 y, uint8 color, const uint8 *chr) {
	dstBuf += dstPitch * y + x;
	uint16 j, i;
	for (j = 0; j < 8; ++j) {
		uint8 *p = dstBuf;
		uint8 c = *chr++;
		if (c != 0) {
			for (i = 0; i < 8; ++i) {
				if(c & 0x80) {
					*p = color;
				}
				++p;
				c <<= 1;
			}
		}
		dstBuf += dstPitch;
	}
}



Display::Display(QueenEngine *vm, Language language, OSystem *system)
	: _system(system), _vm(vm), _horizontalScroll(0), _curBlankingEffect(0) {
	_dynalum.prevColMask = 0xFF;
	_textRenderer._lang = language;
	_textRenderer.init();

	_buffer[RB_BACKDROP] = new uint8[BACKDROP_W * BACKDROP_H];
	_buffer[RB_PANEL]    = new uint8[PANEL_W * PANEL_H];
	_buffer[RB_SCREEN]   = new uint8[SCREEN_W * SCREEN_H];
	_buffer[RB_MINI]     = new uint8[MINI_W * MINI_H];
	memset(_buffer[RB_BACKDROP], 0, BACKDROP_W * BACKDROP_H);
	memset(_buffer[RB_PANEL],    0, PANEL_W * PANEL_H);
	memset(_buffer[RB_SCREEN],   0, SCREEN_W * SCREEN_H);
	memset(_buffer[RB_MINI],     0, MINI_W * MINI_H);
	_bufPitch[RB_BACKDROP] = BACKDROP_W;
	_bufPitch[RB_PANEL]    = PANEL_W;
	_bufPitch[RB_SCREEN]   = SCREEN_W;
	_bufPitch[RB_MINI]     = MINI_W;

	_pal.room   = new uint8[ 256 * 3 ];
	_pal.screen = new uint8[ 256 * 3 ];
	memset(_pal.room,   0, 256 * 3);
	memset(_pal.screen, 0, 256 * 3);
	_pal.dirtyMin = 0;
	_pal.dirtyMax = 255;
	_pal.scrollable = true;
}


Display::~Display() {
	delete[] _buffer[RB_BACKDROP];
	delete[] _buffer[RB_PANEL];
	delete[] _buffer[RB_SCREEN];
	delete[] _buffer[RB_MINI];

	delete[] _pal.room;
	delete[] _pal.screen;
}


void Display::dynalumInit(const char *roomName, uint16 roomNum) {
	debug(9, "Display::dynalumInit(%s, %d)", roomName, roomNum);
	memset(_dynalum.msk, 0, sizeof(_dynalum.msk));
	memset(_dynalum.lum, 0, sizeof(_dynalum.lum));
	_dynalum.valid = false;
	_dynalum.prevColMask = 0xFF;

	if (!(IS_ALT_INTRO_ROOM(roomNum) || IS_CD_INTRO_ROOM(roomNum))) {
		char filename[20];

		sprintf(filename, "%s.msk", roomName);
		_dynalum.valid = _vm->resource()->fileExists(filename);
		if (_dynalum.valid)
			_vm->resource()->loadFile(filename, 0, (uint8*)_dynalum.msk);

		sprintf(filename, "%s.lum", roomName);
		_dynalum.valid = _vm->resource()->fileExists(filename);
		if (_dynalum.valid)
			_vm->resource()->loadFile(filename, 0, (uint8*)_dynalum.lum);
	}
}


void Display::dynalumUpdate(int16 x, int16 y) {
	if (!_dynalum.valid)
		return;

	if (x < 0) {
		x = 0;
	} else if (x >= _bdWidth) {
		x = _bdWidth;
	}
	if (y < 0) {
		y = 0;
	} else if (y >= ROOM_ZONE_HEIGHT - 1) {
		y = ROOM_ZONE_HEIGHT - 1;
	}

	uint offset = (y / 4) * 160 + (x / 4);
	if (offset >= sizeof(_dynalum.msk)) {
		debug(6, "Graphics::dynalumUpdate(%d, %d) - invalid offset: %08x", x, y, offset);
		return;
	}

	uint8 colMask = _dynalum.msk[offset];
	debug(9, "Display::dynalumUpdate(%d, %d) - colMask = %d", x, y, colMask);
	if (colMask != _dynalum.prevColMask) {
		uint8 i;
		for (i = 144; i < 160; ++i) {
			uint8 j;
			for (j = 0; j < 3; ++j) {
				int16 c = (int16)(_pal.room[i * 3 + j] + _dynalum.lum[colMask * 3 + j] * 4);
				if (c < 0) {
					c = 0;
				} else if (c > 255) {
					c = 255;
				}
				_pal.screen[i * 3 + j] = (uint8)c;
			}
		}
		_pal.dirtyMin = MIN(_pal.dirtyMin, 144);
		_pal.dirtyMax = MAX(_pal.dirtyMax, 159);
		_dynalum.prevColMask = colMask;
	}
}


void Display::palConvert(uint8 *outPal, const uint8 *inPal, int start, int end) {
	int i;
	for (i = start; i <= end; i++) {
		outPal[4 * i + 0] = inPal[3 * i + 0];
		outPal[4 * i + 1] = inPal[3 * i + 1];
		outPal[4 * i + 2] = inPal[3 * i + 2];
		outPal[4 * i + 3] = 0;
	}
}


void Display::palSet(const uint8 *pal, int start, int end, bool updateScreen) {
	debug(9, "Display::palSet(%d, %d)", start, end);
	uint8 tempPal[256 * 4];
	palConvert(tempPal, pal, start, end);
	_system->set_palette(tempPal + start * 4, start, end - start + 1);
	if (updateScreen) {
		_system->update_screen();
		waitForTimer();
	}
}


void Display::palSetJoe(JoePalette pal) {
	debug(9, "Display::palSetJoe(%d)", pal);
	const uint8 *palJoe = NULL;
	switch (pal) {
	case JP_CLOTHES:
		palJoe = _palJoeClothes;
		break;
	case JP_DRESS:
		palJoe = _palJoeDress;
		break;
	}
	memcpy(_pal.room + 144 * 3, palJoe, 16 * 3);
	memcpy(_pal.screen + 144 * 3, palJoe, 16 * 3); 
	palSet(_pal.screen, 144, 159, true);
}


void Display::palFadeIn(int start, int end, uint16 roomNum, bool dynalum, int16 dynaX, int16 dynaY) {
	debug(9, "Display::palFadeIn(%d, %d)", start, end);
	memcpy(_pal.screen, _pal.room, 256 * 3);
	if (!(IS_ALT_INTRO_ROOM(roomNum) || IS_CD_INTRO_ROOM(roomNum))) {
		if (dynalum) {
			dynalumUpdate(dynaX, dynaY);
		}
		int n = end - start + 1;
		uint8 tempPal[256 * 3];
		int i;
		for (i = 0; i <= FADE_SPEED; ++i) {
			int j = n * 3;
			uint8 *outPal = tempPal + start * 3;
			const uint8 *inPal = _pal.screen + start * 3;
			while (j--) {
				*outPal = *inPal * i / FADE_SPEED;
				++outPal;
				++inPal;
			}
			palSet(tempPal, start, end, true);
		}
	}
	_pal.dirtyMin = 0;
	_pal.dirtyMax = IS_CD_INTRO_ROOM(roomNum) ? 255 : 223;
	_pal.scrollable = true;
}


void Display::palFadeOut(int start, int end, uint16 roomNum) {
	debug(9, "Display::palFadeOut(%d, %d)", start, end);
	_pal.scrollable = false;
	int n = end - start + 1;
	if (IS_ALT_INTRO_ROOM(roomNum) || IS_CD_INTRO_ROOM(roomNum)) {
		memset(_pal.screen + start * 3, 0, n * 3);
		palSet(_pal.screen, start, end, true);
	} else {
		uint8 tempPal[256 * 3];
		memcpy(tempPal + start * 3, _pal.screen + start * 3, n * 3);
		int i;
		for (i = FADE_SPEED; i >= 0; --i) {
			int j = n * 3;
			uint8 *outPal = _pal.screen + start * 3;
			const uint8 *inPal = tempPal + start * 3;
			while (j--) {
				*outPal = *inPal * i / FADE_SPEED;
				++outPal;
				++inPal;
			}
			palSet(_pal.screen, start, end, true);
		}
	}
}


void Display::palFadePanel() {
	int i;
	uint8 tempPal[256 * 3];
	for (i = 224 * 3; i < 256 * 3; i += 3) {
		uint8 *p = tempPal + i;
		*(p) = *(p + 1) = *(p + 2) = _pal.screen[i + 1] * 2 / 3;
	}

	palSet(tempPal, 224, 255, true);
}


void Display::palScroll(int start, int end) {
	debug(9, "Display::palScroll(%d, %d)", start, end);

	uint8 *palEnd = _pal.screen + end * 3;
	uint8 *palStart = _pal.screen + start * 3;

	uint8 r = *palEnd++;
	uint8 g = *palEnd++;
	uint8 b = *palEnd;

	int n = (end - start) * 3;
	while (n--) {
		*palEnd = *(palEnd - 3);
		--palEnd;
	}

	*palStart++ = r;
	*palStart++ = g;
	*palStart   = b;
}


void Display::palCustomColors(uint16 roomNum) {
	debug(9, "Display::palCustomColors(%d)", roomNum);
	int i;
	switch (roomNum) {
	case 31:
		for(i = 72; i < 84; i++) {
			_pal.room[i * 3 + 1] = _pal.room[i * 3 + 1] * 90 / 100;
			_pal.room[i * 3 + 2] = _pal.room[i * 3 + 2] * 70 / 100;
		}
		break;
	case 29:
		for(i = 72; i < 84; i++) {
			_pal.room[i * 3 + 1] = _pal.room[i * 3 + 1] * 60 / 100;
			_pal.room[i * 3 + 2] = _pal.room[i * 3 + 2] * 60 / 100;
		}
		break;
	case 30:
		for(i = 72; i < 84; i++) {
			_pal.room[i * 3 + 0] = _pal.room[i * 3 + 0] * 60 / 100;
			_pal.room[i * 3 + 1] = _pal.room[i * 3 + 1] * 80 / 100;
		}
		break;
	case 28:
		for(i = 72; i < 84; i++) {
			_pal.room[i * 3 + 0] = _pal.room[i * 3 + 0] * 80 / 100;
			_pal.room[i * 3 + 2] = _pal.room[i * 3 + 1] * 60 / 100;
		}
		break;
	}
}


void Display::palCustomScroll(uint16 roomNum) {
	debug(9, "Display::palCustomScroll(%d)", roomNum);
	static int16 scrollx = 0;

	if (!_pal.scrollable) {
		return;
	}

	int hiPal = 0;
	int loPal = 255;
	int i;

	++scrollx;
	switch (roomNum) {
	case 123: {
			static int16 j = 0, jdir = 2;
			for(i = 96; i < 111; ++i) {
				_pal.screen[i * 3 + 0] = MIN(255, _pal.room[i * 3 + 0] + j * 8);
				_pal.screen[i * 3 + 1] = MIN(255, _pal.room[i * 3 + 1] + j * 4);
			}
			j += jdir;
			if(j <= 0 || j >= 18) {
				jdir = -jdir;
			}
			loPal = 96; 
			hiPal = 111;
		}
		break;
	case 124: {
			static int16 j = 0,jdir = 2;
			for(i = 80; i < 144; ++i) {
				_pal.screen[i * 3 + 0] = MIN(255, _pal.room[i * 3 + 0] + j * 8);
				_pal.screen[i * 3 + 1] = MIN(255, _pal.room[i * 3 + 1] + j * 4);
			}
			j += jdir;
			if(j <= 0 || j >= 14) {
				jdir = -jdir;
				if (_vm->randomizer.getRandomNumber(1)) {
					if (ABS(jdir) == 1) {
						jdir *= 2;
					} else {
						jdir /= 2;
					}
				}
			}
			loPal = 80;
			hiPal = 143;
		}
		break;
	case 125:
		palScroll(32, 63);
		palScroll(64, 95);
		loPal = 32;
		hiPal = 95;
		break;
	case 100:
		if(scrollx & 1) {
			palScroll(128, 132);
			palScroll(133, 137);
			palScroll(138, 143);
			loPal = 128;
			hiPal = 143;
		}
		break;
	case 102:
		if(scrollx & 1) {
			palScroll(112, 127);
			loPal = 112;
			hiPal = 127;
		}
		break;
	case 62:
		if(scrollx & 1) {
			palScroll(108, 119);
			loPal = 108;
			hiPal = 119;
		}
		break;
	case 25:
		palScroll(116, 123);
		loPal = 116;
		hiPal = 123;
		break;
	case 59:
		if(scrollx & 1) {
			palScroll(56, 63);
			loPal = 56;
			hiPal = 63;
		}
		break;
	case 39:
		palScroll(112, 143);
		loPal = 112;
		hiPal = 143;
		break;
	case 74:
		palScroll(28, 31);
		palScroll(88, 91);
		palScroll(92, 95);
		palScroll(128, 135);
		if(scrollx & 1) {
			palScroll(136, 143);
		}
		loPal = 28;
		hiPal = 143;
		break;
	case 40:
		if(scrollx & 1) {
			palScroll(96, 103);
		}
		if(scrollx & 3) {
			palScroll(104, 107);
		}
		loPal = 96;
		hiPal = 107;
		break;
	case 97:
		if(scrollx & 1) {
			palScroll(96, 107);
			palScroll(108, 122);
			loPal = 96;
			hiPal = 122;
		}
		break;
	case 55:
		palScroll(128, 143);
		loPal = 128;
		hiPal = 143;
		break;
	case 57:
		palScroll(128, 143);
		if(scrollx & 1) {
			palScroll(96, 103);
		}
		loPal = 96;
		hiPal = 143;
		break;
	case 76:
		palScroll(88, 95);
		loPal = 88;
		hiPal = 95;
		break;
	case 2:
		if(scrollx & 1) {
			palScroll(120, 127);
			loPal = 120;
			hiPal = 127;
		}
		break;
	case 3:
	case 5:
		if(scrollx & 1) {
			palScroll(128, 135);
			palScroll(136, 143);
			loPal = 128;
			hiPal = 143;
		}
		break;
	case 7:
		if(scrollx & 1) {
			palScroll(119, 127);
			loPal = 119;
			hiPal = 127;
		}
		break;
	case 42:
		if(scrollx & 1) {
			palScroll(118, 127);
			palScroll(136, 143);
			loPal = 118;
			hiPal = 143;
		}
		break;
	case 4:
		if(scrollx & 1) {
			palScroll(32,47);
		}
		palScroll(64, 70);
		palScroll(71, 79);
		loPal = 32;
		hiPal = 79;
		break;
	case 8:
		if(scrollx & 1) {
			palScroll(120, 127);
		}
		loPal = 120;
		hiPal = 127;
		break;
	case 12:
	case 64:
		if(scrollx & 1) {
			palScroll(112, 119);
			palScroll(120, 127);
			loPal = 112;
			hiPal = 127;
		}
		break;
	case 49:
		palScroll(101, 127);
		loPal = 101;
		hiPal = 127;
		break;
	}
	_pal.dirtyMin = MIN(_pal.dirtyMin, loPal);
	_pal.dirtyMax = MAX(_pal.dirtyMax, hiPal);
}


void Display::palCustomFlash() {
	uint8 tempPal[256 * 3];
	int i = 0;
	while (i < 17 * 3) {
		tempPal[i] = 255;
		++i;
	}
	while (i < 84 * 3) {
		tempPal[i] = 0;
		++i;
	}
	while (i < 256 * 3) {
		tempPal[i] = 255;
		++i;
	}
	// set flash palette
	palSet(tempPal, 0, 255, true);
	// restore original palette
	palSet(_pal.screen, 0, 255, true);
}


void Display::palCustomLightsOff(uint16 roomNum) {
	int end = 223;
	int start = (roomNum == ROOM_FLODA_FRONTDESK) ? 32 : 16;
	int n = end - start + 1;

	memset(_pal.screen + start * 3, 0, n * 3);
	palSet(_pal.screen, start, end, true);

	_pal.scrollable = false;
}


void Display::palCustomLightsOn(uint16 roomNum) {
	int end = 223;
	int start = (roomNum == ROOM_FLODA_FRONTDESK) ? 32 : 0;
	int n = end - start + 1;

	memcpy(_pal.screen + start * 3, _pal.room + start * 3, n * 3);
	palSet(_pal.screen, start, end, true);
	
	_pal.dirtyMin = 0;
	_pal.dirtyMax = 223;
	_pal.scrollable = true;
}


void Display::screenMode(int comPanel, bool inCutaway) {
	debug(6, "Display::screenMode(%d, %d)", comPanel, inCutaway);

	// FIXME: this is temporary, just to see if my theory is right
	if (comPanel == 2 && !inCutaway) {
		warning("Display::screenMode() - (comPanel == 2 && !inCutaway)");
	}

	if (comPanel == 2 && inCutaway) {
		_fullscreen = (_bdHeight == GAME_SCREEN_HEIGHT);
	} else if (comPanel == 1) {
		_fullscreen = false;
	}
}


void Display::prepareUpdate() {
	if (!_fullscreen) {
		// draw the panel
		memcpy(_buffer[RB_SCREEN] + _bufPitch[RB_SCREEN] * ROOM_ZONE_HEIGHT, 
			_buffer[RB_PANEL], PANEL_W * PANEL_H);
	}

	// draw the backdrop bitmap
	int i;
	int n = _fullscreen ? 200 : 150;
	uint8 *dst = _buffer[RB_SCREEN];
	uint8 *src = _buffer[RB_BACKDROP] + _horizontalScroll;
	for (i = 0; i < n; ++i) {
		memcpy(dst, src, _bufPitch[RB_SCREEN]);
		dst += _bufPitch[RB_SCREEN];
		src += _bufPitch[RB_BACKDROP];
	}
}


void Display::update(bool dynalum, int16 dynaX, int16 dynaY) {
	if (_pal.scrollable && dynalum) {
		dynalumUpdate(dynaX, dynaY);
	}
	if (_pal.dirtyMin != 144 || _pal.dirtyMax != 144) {
		palSet(_pal.screen, _pal.dirtyMin, _pal.dirtyMax);
		_pal.dirtyMin = 144;
		_pal.dirtyMax = 144;
	}
	drawScreen();
}


void Display::blit(RenderingBuffer dst, uint16 dstX, uint16 dstY, const uint8 *srcBuf, uint16 srcW, uint16 srcH, uint16 srcPitch, bool xflip, bool masked) {
	uint16 dstPitch = _bufPitch[dst];
	uint8 *dstBuf = _buffer[dst] + dstPitch * dstY + dstX;

	if (!masked) { // Unmasked always unflipped
		while (srcH--) {
			memcpy(dstBuf, srcBuf, srcW);
			srcBuf += srcPitch;
			dstBuf += dstPitch;
		}
	} else if (!xflip) { // Masked bitmap unflipped
		while (srcH--) {
			int i;
			for(i = 0; i < srcW; ++i) {
				uint8 b = *(srcBuf + i);
				if(b != 0) {
					*(dstBuf + i) = b;
				}
			}
			srcBuf += srcPitch;
			dstBuf += dstPitch;
		}
	} else { // Masked bitmap flipped
		while (srcH--) {
			int i;
			for(i = 0; i < srcW; ++i) {
				uint8 b = *(srcBuf + i);
				if(b != 0) {
					*(dstBuf - i) = b;
				}
			}
			srcBuf += srcPitch;
			dstBuf += dstPitch;   
		}
	}
}


void Display::fill(RenderingBuffer dst, uint16 x, uint16 y, uint16 w, uint16 h, uint8 color) {
	assert(w <= _bufPitch[dst]);
	uint16 dstPitch = _bufPitch[dst];
	uint8 *dstBuf = _buffer[dst] + dstPitch * y + x;
	while (h--) {
		memset(dstBuf, color, w);
		dstBuf += dstPitch;
	}
}


void Display::readPCX(uint8 *dst, uint16 dstPitch, const uint8 *src, uint16 w, uint16 h) {
	while (h--) {
		uint8 *p = dst;
		while (p < dst + w ) {
			uint8 col = *src++;
			if ((col & 0xC0) == 0xC0) {
				uint8 len = col & 0x3F;
				memset(p, *src++, len);
				p += len;
			} else {
				*p++ = col;
			}
		}
		dst += dstPitch;
	}
}


void Display::readPCXBackdrop(const uint8 *pcxBuf, uint32 size, bool useFullPal) {
	_bdWidth  = READ_LE_UINT16(pcxBuf + 12);
	_bdHeight = READ_LE_UINT16(pcxBuf + 14);
	readPCX(_buffer[RB_BACKDROP], _bufPitch[RB_BACKDROP], pcxBuf + 128, _bdWidth, _bdHeight);
	memcpy(_pal.room, pcxBuf + size - 768, useFullPal ? 256 * 3 : 144 * 3);
}


void Display::readPCXPanel(const uint8 *pcxBuf, uint32 size) {
	uint8 *dst = _buffer[RB_PANEL] + PANEL_W * 10;
	readPCX(dst, PANEL_W, pcxBuf + 128, PANEL_W, PANEL_H - 10);
	memcpy(_pal.room + 144 * 3, pcxBuf + size - 768 + 144 * 3, (256 - 144) * 3);
}


void Display::horizontalScrollUpdate(int16 xCamera) {
	debug(9, "Display::horizontalScrollUpdate(%d)", xCamera);
	_horizontalScroll = 0;
	if (_bdWidth > 320) {
		if (xCamera > 160 && xCamera < 480) {
			_horizontalScroll = xCamera - 160;
		} else if (xCamera >= 480) {
			_horizontalScroll = 320;
		}
	}
}


void Display::horizontalScroll(int16 scroll) {
	_horizontalScroll = scroll;
}


void Display::handleTimer() {
	_gotTick = true;
}


void Display::waitForTimer() {
	_gotTick = false;
	while (!_gotTick) {
		_vm->input()->delay(10);
	}
}


void Display::setMouseCursor(uint8 *buf, uint16 w, uint16 h, uint16 xhs, uint16 yhs) {
	// change transparency color match the one expected by the backend (0xFF)
	uint16 size = w * h;
	uint8 *p = buf;
	while (size--) {
		if (*p == 255) {
			*p = 254;
		} else if (*p == 0) {
			*p = 255;
		}
		++p;
	}
	_system->set_mouse_cursor(buf, w, h, xhs, yhs);
}


void Display::showMouseCursor(bool show) {
	_system->show_mouse(show);
}


uint16 Display::textWidth(const char *text) const {
	uint16 len = 0;
	while (*text) {
		len += _textRenderer._charWidth[ (uint8)*text ];
		++text;
	}
	return len;
}


void Display::drawText(uint16 x, uint16 y, uint8 color, const char *text, bool outlined) {
	debug(9, "Display::drawText(%s)", text);
	_textRenderer.drawString(_buffer[RB_SCREEN], _bufPitch[RB_SCREEN], x, y, color, text, outlined);
}


void Display::drawBox(int16 x1, int16 y1, int16 x2, int16 y2, uint8 col) {
	uint8 *p = _buffer[RB_SCREEN];
	uint16 pitch = _bufPitch[RB_SCREEN];

	int i;
	for (i = y1; i <= y2; ++i) {
		*(p + i * pitch + x1) = *(p + i * pitch + x2) = col;
	}
	for (i = x1; i <= x2; ++i) {
		*(p + y1 * pitch + i) = *(p + y2 * pitch + i) = col;
	}
}


void Display::drawScreen() {
	_system->copy_rect(_buffer[RB_SCREEN], _bufPitch[RB_SCREEN], 0, 0, SCREEN_W, SCREEN_H);
	_system->update_screen();
	waitForTimer();
}



void Display::blankScreen() {
	typedef void (Display::*BlankerEffect)();
	static const BlankerEffect effects[] = {
		&Display::blankScreenEffect1,
		&Display::blankScreenEffect2,
		&Display::blankScreenEffect3
	};
	(this->*effects[_curBlankingEffect])();
	_curBlankingEffect = (_curBlankingEffect + 1) % 3;
}


void Display::blankScreenEffect1() {
	while (_vm->input()->idleTime() >= Input::DELAY_SCREEN_BLANKER) {
		for(int i = 0; i < 2; ++i) {    
			uint16 x = _vm->randomizer.getRandomNumber(SCREEN_W - MINI_W - 2) + 1;
			uint16 y = _vm->randomizer.getRandomNumber(SCREEN_H - MINI_H - 2) + 1;
			uint8 *p = _buffer[RB_SCREEN] + _bufPitch[RB_SCREEN] * y + x;
			blit(RB_MINI, 0, 0, p, MINI_W, MINI_H, _bufPitch[RB_SCREEN], false, false);
			if (_vm->randomizer.getRandomNumber(1)) {
				--x;
			} else {
				++x;
			}
			if (_vm->randomizer.getRandomNumber(1)) {
				--y;
			} else {
				++y;
			}
			blit(RB_SCREEN, x, y, _buffer[RB_MINI], MINI_W, MINI_H, _bufPitch[RB_MINI], false, false);
			drawScreen();
		}
	}
}


void Display::blankScreenEffect2() {
	while (_vm->input()->idleTime() >= Input::DELAY_SCREEN_BLANKER) {
		uint16 x = _vm->randomizer.getRandomNumber(SCREEN_W - 2);
		uint16 y = _vm->randomizer.getRandomNumber(SCREEN_H - 2);
		uint8 c = 0;
		switch (_vm->randomizer.getRandomNumber(3)) {
		case 0:
			c = *(_buffer[RB_SCREEN] + _bufPitch[RB_SCREEN] * y + x);
			break;
		case 1:
			c = *(_buffer[RB_SCREEN] + _bufPitch[RB_SCREEN] * y + x + 1);
			break;
		case 2:
			c = *(_buffer[RB_SCREEN] + _bufPitch[RB_SCREEN] * (y + 1) + x);
			break;
		case 3:
			c = *(_buffer[RB_SCREEN] + _bufPitch[RB_SCREEN] * (y + 1) + x + 1);
			break;
		default:
			break;
		}
		uint8 *p = _buffer[RB_SCREEN] + y * _bufPitch[RB_SCREEN] + x;
		int j = 2;
		while (j--) {
			memset(p, c, 2);
			p += _bufPitch[RB_SCREEN];
		}
		drawScreen();
	}
}


void Display::blankScreenEffect3() {
	uint32 i = 0;
	while (_vm->input()->idleTime() >= Input::DELAY_SCREEN_BLANKER) {
		uint16 x = _vm->randomizer.getRandomNumber(SCREEN_W - 2);
		uint16 y = _vm->randomizer.getRandomNumber(SCREEN_H - 2);
		uint8 *p = _buffer[RB_SCREEN] + _bufPitch[RB_SCREEN] * y + x;
		uint8 p0 = *p;
		uint8 p1 = *(p + 1);
		uint8 p2 = *(p + _bufPitch[RB_SCREEN]);
		uint8 p3 = *(p + _bufPitch[RB_SCREEN] + 1);
		uint8 c = (p0 + p1 + p2 + p3) / 4;
		int j = 2;
		while (j--) {
			memset(p, c, 2);
			p += _bufPitch[RB_SCREEN];
		}
		if (i > 4000000) {
			memset(_buffer[RB_SCREEN], 0, SCREEN_W * SCREEN_H);
		}
		drawScreen();
		++i;
	}
}


const uint8 TextRenderer::_font[] = {
	0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 
	0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 
	0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 
	0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 
	0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 
	0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 
	0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 
	0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 
	0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 
	0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 
	0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 
	0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 
	0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 
	0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 
	0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 
	0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 
	0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 
	0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 
	0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 
	0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 
	0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 
	0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 
	0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 
	0xC0, 0x00, 0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 
	0xF8, 0xB0, 0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0xF8, 0xB0, 
	0xB0, 0x80, 0xB0, 0xB0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xC0, 0x00, 0xC0, 
	0xC0, 0x00, 0xD8, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00, 0x30, 0x7C, 
	0xC0, 0x78, 0x0C, 0xF8, 0x30, 0x00, 0x00, 0xC6, 0xCC, 0x18, 
	0x30, 0x66, 0xC6, 0x00, 0x38, 0x6C, 0x68, 0x36, 0xDC, 0xCC, 
	0x76, 0x00, 0x60, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x30, 0x60, 0xC0, 0xC0, 0xC0, 0x60, 0x30, 0x00, 0xC0, 0x60, 
	0x30, 0x30, 0x30, 0x60, 0xC0, 0x00, 0x00, 0x6C, 0x38, 0xFE, 
	0x38, 0x6C, 0x00, 0x00, 0x00, 0x30, 0x30, 0xFC, 0x30, 0x30, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0xC0, 
	0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x03, 0x06, 0x0C, 0x18, 
	0x30, 0x60, 0xC0, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 
	0x78, 0x00, 0x30, 0x70, 0xF0, 0x30, 0x30, 0x30, 0x30, 0x00, 
	0x78, 0xCC, 0x0C, 0x78, 0xC0, 0xC0, 0xFC, 0x00, 0x78, 0xCC, 
	0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00, 0x1C, 0x3C, 0x6C, 0xCC, 
	0xFC, 0x0C, 0x0C, 0x00, 0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 
	0x78, 0x00, 0x78, 0xCC, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00, 
	0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00, 0x78, 0xCC, 
	0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00, 0x78, 0xCC, 0xCC, 0x7C, 
	0x0C, 0xCC, 0x78, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0xC0, 
	0xC0, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x60, 0xC0, 
	0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00, 0x00, 0x00, 
	0xFC, 0x00, 0xFC, 0x00, 0x00, 0x00, 0xC0, 0x60, 0x30, 0x18, 
	0x30, 0x60, 0xC0, 0x00, 0x78, 0xCC, 0x0C, 0x18, 0x30, 0x00, 
	0x30, 0x00, 0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00, 
	0x38, 0x7C, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0x00, 0xF8, 0xCC, 
	0xCC, 0xF8, 0xCC, 0xCC, 0xF8, 0x00, 0x78, 0xCC, 0xC0, 0xC0, 
	0xC0, 0xCC, 0x78, 0x00, 0xF8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 
	0xF8, 0x00, 0xFC, 0xC0, 0xC0, 0xF0, 0xC0, 0xC0, 0xFC, 0x00, 
	0xFC, 0xC0, 0xC0, 0xF0, 0xC0, 0xC0, 0xC0, 0x00, 0x78, 0xCC, 
	0xC0, 0xDC, 0xCC, 0xCC, 0x7C, 0x00, 0xCC, 0xCC, 0xCC, 0xFC, 
	0xCC, 0xCC, 0xCC, 0x00, 0xF0, 0x60, 0x60, 0x60, 0x60, 0x60, 
	0xF0, 0x00, 0x0C, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00, 
	0xC6, 0xCC, 0xD8, 0xF8, 0xD8, 0xCC, 0xC6, 0x00, 0xC0, 0xC0, 
	0xC0, 0xC0, 0xC0, 0xC0, 0xFC, 0x00, 0x82, 0xC6, 0xEE, 0xFE, 
	0xD6, 0xC6, 0xC6, 0x00, 0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 
	0xC6, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x00, 
	0xF8, 0xCC, 0xCC, 0xF8, 0xC0, 0xC0, 0xC0, 0x00, 0x78, 0xCC, 
	0xCC, 0xCC, 0xCC, 0xDC, 0x78, 0x0C, 0xF8, 0xCC, 0xCC, 0xF8, 
	0xD8, 0xCC, 0xCC, 0x00, 0x78, 0xCC, 0xC0, 0x78, 0x0C, 0xCC, 
	0x78, 0x00, 0xFC, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 
	0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x7C, 0x00, 0xC6, 0xC6, 
	0x6C, 0x6C, 0x38, 0x38, 0x10, 0x00, 0xC6, 0xC6, 0xC6, 0xD6, 
	0xFE, 0xEE, 0xC6, 0x00, 0xC6, 0x6C, 0x38, 0x10, 0x38, 0x6C, 
	0xC6, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x30, 0x00, 
	0xFC, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0xFC, 0x00, 0xF0, 0xC0, 
	0xC0, 0xC0, 0xC0, 0xC0, 0xF0, 0x00, 0xC0, 0x60, 0x30, 0x18, 
	0x0C, 0x06, 0x03, 0x00, 0xF0, 0x30, 0x30, 0x30, 0x30, 0x30, 
	0xF0, 0x00, 0xE8, 0x4D, 0x4A, 0x48, 0x00, 0x00, 0x00, 0x00, 
	0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 
	0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x0C, 
	0x7C, 0xCC, 0x7C, 0x00, 0xC0, 0xC0, 0xF8, 0xCC, 0xCC, 0xCC, 
	0xF8, 0x00, 0x00, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00, 
	0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0xCC, 0x7C, 0x00, 0x00, 0x00, 
	0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00, 0x38, 0x6C, 0x60, 0xF8, 
	0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x7C, 0xCC, 0xCC, 0x7C, 
	0x0C, 0x78, 0xC0, 0xC0, 0xF8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00, 
	0xC0, 0x00, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x00, 0x0C, 0x00, 
	0x0C, 0x0C, 0x0C, 0x0C, 0xCC, 0x78, 0xC0, 0xC0, 0xCC, 0xD8, 
	0xF0, 0xD8, 0xCC, 0x00, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 
	0xC0, 0x00, 0x00, 0x00, 0xCC, 0xEE, 0xD6, 0xC6, 0xC6, 0x00, 
	0x00, 0x00, 0xF8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00, 0x00, 0x00, 
	0x78, 0xCC, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0x00, 0xF8, 0xCC, 
	0xCC, 0xF8, 0xC0, 0xC0, 0x00, 0x00, 0x7C, 0xCC, 0xCC, 0x7C, 
	0x0C, 0x0C, 0x00, 0x00, 0xF8, 0xCC, 0xC0, 0xC0, 0xC0, 0x00, 
	0x00, 0x00, 0x7C, 0xC0, 0x78, 0x0C, 0x78, 0x00, 0x30, 0x30, 
	0xFC, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0xCC, 0xCC, 
	0xCC, 0xCC, 0x7C, 0x00, 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 
	0x30, 0x00, 0x00, 0x00, 0xC6, 0xD6, 0xD6, 0x6C, 0x6C, 0x00, 
	0x00, 0x00, 0xCC, 0x78, 0x30, 0x78, 0xCC, 0x00, 0x00, 0x00, 
	0xCC, 0xCC, 0xCC, 0x78, 0x30, 0xE0, 0x00, 0x00, 0xFC, 0x18, 
	0x30, 0x60, 0xFC, 0x00, 0x38, 0x60, 0x60, 0xC0, 0x60, 0x60, 
	0x38, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 
	0xE0, 0x30, 0x30, 0x18, 0x30, 0x30, 0xE0, 0x00, 0x38, 0x44, 
	0xBA, 0xAA, 0xBA, 0x44, 0x38, 0x00, 0x00, 0x98, 0x30, 0x60, 
	0xC8, 0x98, 0x30, 0x00, 0x1E, 0x30, 0x60, 0x60, 0x30, 0x1E, 
	0x0C, 0x18, 0x00, 0x66, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x00, 
	0x0C, 0x18, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00, 0x18, 0x66, 
	0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x30, 0x18, 0x3C, 0x06, 0x3E, 0x66, 
	0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x1E, 0x30, 0x60, 0x60, 0x30, 0x1E, 0x0C, 0x18, 0x18, 0x66, 
	0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00, 0x66, 0x00, 0x3C, 0x66, 
	0x7E, 0x60, 0x3C, 0x00, 0x30, 0x18, 0x3C, 0x66, 0x7E, 0x60, 
	0x3C, 0x00, 0x00, 0x66, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 
	0x18, 0x66, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x30, 0x18, 
	0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x30, 0xFC, 0xC0, 0xF0, 0xC0, 0xFC, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x66, 0x00, 0x3C, 
	0x66, 0x66, 0x3C, 0x00, 0x00, 0x66, 0x00, 0x3C, 0x66, 0x66, 
	0x3C, 0x00, 0x30, 0x18, 0x00, 0x3C, 0x66, 0x66, 0x3C, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x18, 
	0x00, 0x66, 0x66, 0x66, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x18, 0x30, 0x78, 0x0C, 0x7C, 0xCC, 0x7C, 0x00, 0x0C, 0x18, 
	0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x30, 0x00, 0x78, 
	0xCC, 0xCC, 0x78, 0x00, 0x18, 0x30, 0x00, 0xCC, 0xCC, 0xCC, 
	0x7C, 0x00, 0x71, 0x8E, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x00, 
	0x71, 0xCE, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0x00, 0x18, 0x18, 
	0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 0x3C, 0x60, 0x3C, 0x66, 
	0x3C, 0x06, 0x3C, 0x00, 0x18, 0x00, 0x18, 0x0C, 0x06, 0x66, 
	0x3C, 0x00, 0x3F, 0x40, 0x4E, 0x58, 0x4E, 0x40, 0x3F, 0x00, 
	0x1C, 0xA4, 0xC4, 0xBC, 0x80, 0xFE, 0x00, 0x00, 0x00, 0x33, 
	0x66, 0xCC, 0x66, 0x33, 0x00, 0x00, 0x3E, 0x06, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0xC0, 0xC0, 0xC0, 
	0xC0, 0x00, 0x81, 0xB9, 0xA5, 0xB9, 0xA5, 0x81, 0x7E, 0x00, 
	0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xCC, 
	0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0xFC, 0x30, 
	0x30, 0x00, 0xFC, 0x00, 0xF0, 0x18, 0x30, 0x60, 0xF8, 0x00, 
	0x00, 0x00, 0xF0, 0x18, 0x30, 0x18, 0xF0, 0x00, 0x00, 0x00, 
	0x30, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xCC, 0xCC, 0xCC, 0xCC, 0xFE, 0xC0, 0x3E, 0x7A, 0x7A, 0x3A, 
	0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x60, 
	0x60, 0xE0, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x38, 0x44, 
	0x44, 0x38, 0x00, 0x7C, 0x00, 0x00, 0x00, 0xCC, 0x66, 0x33, 
	0x66, 0xCC, 0x00, 0x00, 0x40, 0xC6, 0x4C, 0x58, 0x32, 0x66, 
	0xCF, 0x02, 0x40, 0xC6, 0x4C, 0x58, 0x3E, 0x62, 0xC4, 0x0E, 
	0xC0, 0x23, 0x66, 0x2C, 0xD9, 0x33, 0x67, 0x01, 0x18, 0x00, 
	0x18, 0x30, 0x60, 0x66, 0x3C, 0x00, 0x60, 0x30, 0x7C, 0xC6, 
	0xFE, 0xC6, 0xC6, 0x00, 0x0C, 0x18, 0x7C, 0xC6, 0xFE, 0xC6, 
	0xC6, 0x00, 0x38, 0xC6, 0x7C, 0xC6, 0xFE, 0xC6, 0xC6, 0x00, 
	0x71, 0x8E, 0x7C, 0xC6, 0xFE, 0xC6, 0xC6, 0x00, 0x6C, 0x00, 
	0x7C, 0xC6, 0xFE, 0xC6, 0xC6, 0x00, 0x38, 0x44, 0x7C, 0xC6, 
	0xFE, 0xC6, 0xC6, 0x00, 0x1F, 0x3C, 0x3C, 0x6F, 0x7C, 0xCC, 
	0xCF, 0x00, 0x1E, 0x30, 0x60, 0x60, 0x30, 0x1E, 0x0C, 0x18, 
	0x60, 0x30, 0xFC, 0xC0, 0xF0, 0xC0, 0xFC, 0x00, 0x18, 0x30, 
	0xFC, 0xC0, 0xF0, 0xC0, 0xFC, 0x00, 0x30, 0xCC, 0xFC, 0xC0, 
	0xF0, 0xC0, 0xFC, 0x00, 0xCC, 0x00, 0xFC, 0xC0, 0xF0, 0xC0, 
	0xFC, 0x00, 0x60, 0x30, 0x78, 0x30, 0x30, 0x30, 0x78, 0x00, 
	0x18, 0x30, 0x78, 0x30, 0x30, 0x30, 0x78, 0x00, 0x30, 0xCC, 
	0x78, 0x30, 0x30, 0x30, 0x78, 0x00, 0xCC, 0x00, 0x78, 0x30, 
	0x30, 0x30, 0x78, 0x00, 0x78, 0x6C, 0x66, 0xF6, 0x66, 0x6C, 
	0x78, 0x00, 0x71, 0xCE, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0x00, 
	0x30, 0x18, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x0C, 0x18, 
	0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x18, 0x66, 0x3C, 0x66, 
	0x66, 0x66, 0x3C, 0x00, 0x71, 0x8E, 0x3C, 0x66, 0x66, 0x66, 
	0x3C, 0x00, 0xC3, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 
	0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x3F, 0x66, 
	0x6E, 0x7E, 0x76, 0x66, 0xFC, 0x00, 0x30, 0x18, 0x66, 0x66, 
	0x66, 0x66, 0x3E, 0x00, 0x0C, 0x18, 0x66, 0x66, 0x66, 0x66, 
	0x3E, 0x00, 0x18, 0x24, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00, 
	0x66, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00, 0x06, 0x08, 
	0xC3, 0x66, 0x3C, 0x18, 0x18, 0x00, 0x60, 0x60, 0x7E, 0x63, 
	0x7E, 0x60, 0x60, 0x00, 0x3C, 0x66, 0x66, 0x6C, 0x66, 0x66, 
	0x6C, 0x60, 0x30, 0x18, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00, 
	0x0C, 0x18, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00, 0x18, 0x66, 
	0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00, 0x71, 0x8E, 0x3C, 0x06, 
	0x3E, 0x66, 0x3E, 0x00, 0x66, 0x00, 0x3C, 0x06, 0x3E, 0x66, 
	0x3E, 0x00, 0x18, 0x24, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00, 
	0x00, 0x00, 0x7E, 0x1B, 0x7F, 0xD8, 0x77, 0x00, 0x00, 0x00, 
	0x3C, 0x60, 0x60, 0x60, 0x3C, 0x18, 0x30, 0x18, 0x3C, 0x66, 
	0x7E, 0x60, 0x3C, 0x00, 0x0C, 0x18, 0x3C, 0x66, 0x7E, 0x60, 
	0x3C, 0x00, 0x18, 0x66, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00, 
	0x66, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00, 0x30, 0x18, 
	0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x0C, 0x18, 0x00, 0x18, 
	0x18, 0x18, 0x18, 0x00, 0x18, 0x66, 0x00, 0x18, 0x18, 0x18, 
	0x18, 0x00, 0x00, 0x66, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 
	0x60, 0xFC, 0x18, 0x3C, 0x66, 0x66, 0x3C, 0x00, 0x71, 0x8E, 
	0x00, 0x7C, 0x66, 0x66, 0x66, 0x00, 0x30, 0x18, 0x00, 0x3C, 
	0x66, 0x66, 0x3C, 0x00, 0x0C, 0x18, 0x00, 0x3C, 0x66, 0x66, 
	0x3C, 0x00, 0x18, 0x66, 0x00, 0x3C, 0x66, 0x66, 0x3C, 0x00, 
	0x71, 0x8E, 0x00, 0x3C, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x66, 
	0x00, 0x3C, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x18, 0x00, 0x7E, 
	0x00, 0x18, 0x00, 0x00, 0x00, 0x02, 0x7C, 0xCE, 0xD6, 0xE6, 
	0x7C, 0x80, 0x30, 0x18, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x00, 
	0x0C, 0x18, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x00, 0x18, 0x66, 
	0x00, 0x66, 0x66, 0x66, 0x3E, 0x00, 0x00, 0x66, 0x00, 0x66, 
	0x66, 0x66, 0x3E, 0x00, 0x0C, 0x18, 0x00, 0x66, 0x66, 0x3C, 
	0x18, 0x30, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 
	0x00, 0x66, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x30, 0xFF 
};


const uint8 Display::_palJoeClothes[] = {
	0x00, 0x00, 0x00, 0x60, 0x60, 0x60, 0x87, 0x87, 0x87, 
	0xB0, 0xB0, 0xB0, 0xDA, 0xDA, 0xDA, 0x43, 0x34, 0x20,
	0x77, 0x33, 0x1F, 0xA3, 0x43, 0x27, 0x80, 0x45, 0x45, 
	0x9E, 0x5D, 0x5B, 0xB9, 0x78, 0x75, 0xDF, 0x97, 0x91, 
	0x17, 0x27, 0x63, 0x1F, 0x3F, 0x83, 0x27, 0x5B, 0xA7,
	0x98, 0xD4, 0xFF
};


const uint8 Display::_palJoeDress[] = {
	0x00, 0x00, 0x00, 0x50, 0x50, 0x50, 0x70, 0x70, 0x70,
	0x90, 0x90, 0x90, 0xC6, 0xC6, 0xC6, 0xFF, 0xFF, 0xFF, 
	0x30, 0x30, 0x90, 0x47, 0x49, 0xD0, 0x40, 0x24, 0x00, 
	0x79, 0x34, 0x0B, 0xB2, 0x3D, 0x22, 0xED, 0x42, 0x42, 
	0x80, 0x45, 0x45, 0xA3, 0x5F, 0x5F, 0xC8, 0x7C, 0x7C, 
	0xEC, 0x9C, 0x9C
};


} // End of namespace Queen
