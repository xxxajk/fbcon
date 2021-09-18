/*
 * File:   fbcon.cpp
 * Author: xxxajk
 *
 * Created on September 12, 2021, 4:11 AM
 */

#include "fbcon.h"

void fbcon::drawbyte(uint16_t x, uint16_t y, uint8_t b, rgb2_t fg) {
        for(uint8_t bit = 0, mask = 0x80U; bit < 8; bit++, mask >>= 1) {
                if((b & mask)) {
                        pixel->set(x + bit, y, fg);
                } else {
                        pixel->clear(x + bit, y, BGcolor);
                }
        }
}

void fbcon::drawSymbol(uint16_t x, uint16_t y, charcell_t s) {
        if(x >= _sizeX) return;
        if(y >= _sizeY) return;
        rgb2_t fg;
        fg.color = s.color;
        bool f = s.flash && state;
        bool i = s.inverse != f;
        uint16_t c = s.c * 8;
        y = y * 8;
        x = x * 8;
        for(uint16_t v = 0; v < 8; v++) {
                uint8_t b = i ? ~symbits[c + v] : symbits[c + v];
                drawbyte(x, y + v, b, fg);
        }
}

void fbcon::updateFB(void) {
        it.pause();
        user = true;
        CISR(); // unpauses for us...
}

void fbcon::FBset(uint16_t x, uint16_t y, uint16_t s) {
        if(x >= _sizeX) return;
        if(y >= _sizeY) return;
        if(s > 1023) return;
        noInterrupts();
        charcell_t *p = FBcon + (x + (y * _sizeX));
        p->cell = s /* | FGcolor.color << 10 */;
        p->color = FGcolor.color;
        interrupts();
}

// to clear the display, we fill it with the space symbol, which is at 0x20 | color, also sets bg color

void fbcon::FBclear(rgb2_t color, rgb2_t bg) {
        FGcolor.color = color.color;
        BGcolor.color = bg.color;
        for(uint16_t y = 0; y < _sizeY; y++) {
                for(uint16_t x = 0; x < _sizeX; x++) {
                        FBset(x, y, 0x20);
                }
        }
        updateFB();
}

void fbcon::scroll(void) {
        noInterrupts();
        memmove((void*)FBcon, (void*)(FBcon + _sizeX), (_sizeB - _sizeX) * sizeof (uint16_t));
        for(uint16_t x = 0; x < _sizeX; x++) {
                FBset(x, _sizeY - 1, 0x20);
        }
        updateFB();
}

size_t fbcon::write(uint8_t c) {
        if(!diag) {
                switch(c) {
                        case 0x00: // ^@
                                break;
                        case 0x01: // ^A home
                                pX = 0;
                                pY = 0;
                                break;
                        case 0x02: // ^B
                                break;
                        case 0x03: // ^C
                                break;
                        case 0x04: // ^D
                                break;
                        case 0x05: // ^E
                                break;
                        case 0x06: // ^F RIGHT
                                if(pX < _sizeX - 1) {
                                        pX++;
                                }
                                break;
                        case 0x07: // ^G /a
                                if(alert) {
                                        alert->alert();
                                }
                                break;
                        case 0x08: // ^H /b
                                if(pX > 0) {
                                        pX--;
                                } else {
                                        pX = _sizeX - 1;
                                        if(pY > 0) {
                                                pY--;
                                        }
                                }
                                break;
                        case 0x09: // ^I /t TAB
                                pX = (pX & ~0x07) + 8;
                                if(pX >= _sizeX) {
                                        pX = 0;
                                        if(pY == _sizeY - 1) {
                                                scroll();
                                        } else {
                                                pY++;
                                        }
                                }
                                break;
                        case 0x0a: // ^J /n DOWN
                                if(pY == _sizeY - 1) {
                                        scroll();
                                } else {
                                        pY++;
                                }
                                break;
                        case 0x0b: // ^K
                                break;
                        case 0x0c: // ^L CLEAR/HOME
                                pX = 0;
                                pY = 0;
                                FBclear(FGcolor, BGcolor);
                                break;
                        case 0x0d: // ^M /r
                                pX = 0;
                                break;
                        case 0x0e: // ^N
                                break;
                        case 0x0f: // ^O
                                break;
                        case 0x10: // ^P
                                break;
                        case 0x11: // ^Q
                                break;
                        case 0x12: // ^R
                                break;
                        case 0x13: // ^S
                                break;
                        case 0x14: // ^T
                                break;
                        case 0x15: // ^U LEFT
                                if(pX > 0) {
                                        pX--;
                                }
                                break;
                        case 0x16: // ^V
                                break;
                        case 0x17: // ^W
                                break;
                        case 0x18: // ^X
                                break;
                        case 0x19: // ^Y
                                break;
                        case 0x1a: // ^Z UP
                                if(pY > 0) {
                                        pY--;
                                }
                                break;
                        case 0x1b: // ^[
                                break;
                        case 0x1c: // ^BACKSLASH
                                break;
                        case 0x1d: // ^]
                                break;
                        case 0x1e: // ^^
                                break;
                        case 0x1f: // ^_
                                break;
                        default:
                                // emit it as-is, and only advance line if we go beyond
                                // this allows having a character at the end of the line
                                // and returning to the beginning/not scrolling
                                uint16_t ch = c | (inv ? 0x100 : 0) | (fla ? 0x200 : 0);
                                if(pX == _sizeX) {
                                        pX = 0;
                                        pY++;
                                        if(pY == _sizeY) {
                                                pY--;
                                                scroll();
                                        }
                                        FBset(pX, pY, ch);
                                } else {
                                        FBset(pX, pY, ch);
                                        pX++;
                                }
                                break;
                }
        } else {
                // diagnostic mode, for viewing raw control chars
                uint16_t ch = c | (inv ? 0x100 : 0) | (fla ? 0x200 : 0);
                FBset(pX, pY, ch);
                pX++;
                if(pX == _sizeX) {
                        pY++;
                        if(pY == _sizeY) {
                                pY--;
                                scroll();
                        }
                }
        }
        return 1;
}

void fbcon::CISR(void) {
        it.pause();
        if(!user) { // user called, so don't update flashing state
                state = !state;
        }
        charcell_t *cell = FBcon;
        charcell_t *cellcopy = FBcpy;
        for(int i = 0; i < _sizeB; i++, cell++, cellcopy++) {
                if((cell->cell != cellcopy->cell) || (cellcopy->flash && !user)) {
                        *cellcopy = *cell;
                        uint16_t y = i / _sizeX;
                        uint16_t x = i - (y * _sizeX);
                        drawSymbol(x, y, *cellcopy);
                }
        }
        pixel->commit();
        if((_posX != pX) || (_posY != pY)) {
                // draw normal char @ cursor pos
                uint16_t x = _posX == _sizeX ? _sizeX - 1 : _posX;
                cell = FBcon + (x + (_posY * _sizeX));
                drawSymbol(x, _posY, *cell);
                _posX = pX;
                _posY = pY;
        }
        if(flash) {
                uint16_t x = _posX == _sizeX ? _sizeX - 1 : _posX;
                cell = FBcon + (x + (_posY * _sizeX));
                charcell_t e = *cell;
                e.flash = true;
                e.inverse = !e.inverse;
                drawSymbol(x, _posY, e);
        }
        user = false;
        it.resume(/* this, 500000UL */);
}

void fbcon::begin(uint16_t x, uint16_t y, fbcon_pixel *_pixel, fbcon_alert *_alert) {
        if(!initted) {
                symbits = font_CODERMONO;
                _sizeX = x / 8;
                _sizeY = y / 8;
                _posX = 0;
                _posY = 0;
                pX = 0;
                pY = 0;
                flash = false;
                state = false;
                user = false;
                inv = false;
                fla = false;
                diag = false;
                pixel = _pixel;
                alert = _alert;
                _sizeB = _sizeY * _sizeX;
                FBcon = new charcell_t[_sizeB];
                FBcpy = new charcell_t[_sizeB];
                FGcolor.color = FB_WHITE.color;
                BGcolor.color = FB_BLACK.color;

                // init frame buffers;
                charcell_t *p = FBcon;
                charcell_t *q = FBcpy;
                for(int i = 0; i < _sizeB; i++, p++, q++) {
                        p->cell = 0x007FU | FB_WHITE.color << 10;
                        q->cell = 0x0001U | FB_WHITE.color << 10;
                }
                it.priority(255); // use lowest priority, allow more important events to happen
                it.begin(this, 500000UL);
                updateFB();
                FBclear(FB_WHITE, FB_BLACK); // clear display
                initted = true;
        }
}

// RGB222 -> monochrome

uint8_t FB_to_monochrome(rgb2_t color) {
        // NOTE: the "color" is reversed on back-lit/reflective LCD,
        //       but is correct for LED displays.
        return (color.color) ? 1 : 0;
}

// RGB222 -> RGB565 LUTs
static const uint16_t FB_lt222_xx5[4] = {0x0000U, 0x0007U, 0x001CU, 0x001FU}; // B
static const uint16_t FB_lt222_x6x[4] = {0x0000U, 0x00E0U, 0x0700U, 0x07E0U}; // G
static const uint16_t FB_lt222_5xx[4] = {0x0000U, 0x3800U, 0xC000U, 0xF800U}; // R
// RGB222 -> RGB565

uint16_t FB_to_565(rgb2_t color) {
        return (FB_lt222_5xx[color.r] | FB_lt222_x6x[color.g] | FB_lt222_xx5[color.b]);
}
