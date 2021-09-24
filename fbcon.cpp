/*
 * File:   fbcon.cpp
 * Author: xxxajk
 *
 * Created on September 12, 2021, 4:11 AM
 */

#include "fbcon.h"

void fbcon::drawbyte(uint16_t x, uint16_t y, uint8_t b, rgb222_t fg) {
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
        rgb222_t fg;
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
        CISR(); // This resumes for us
}

void fbcon::FBset(uint16_t x, uint16_t y, uint16_t s) {
        if(x >= _sizeX) return;
        if(y >= _sizeY) return;
        if(s > 1023) return;
        noInterrupts();
        charcell_t *p = FBcon + (x + (y * _sizeX));
        p->cell = s /* | FGcolor.color << 10 */;
        p->color = FGcolor.color._color;
        interrupts();
}

// to clear the display, we fill it with the space symbol, which is at 0x20 | color, also sets bg color

void fbcon::FBclear(rgb222_t color, rgb222_t bg) {
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

void fbcon::setprcol(void) {
        switch(prcol) {
                case 0x00:
                        if(bold) {
                                FBfgcolor(FB_COLOR_8);
                        } else {
                                FBfgcolor(FB_COLOR_0);
                        }
                        break;
                case 0x01:
                        if(bold) {
                                FBfgcolor(FB_COLOR_9);
                        } else {
                                FBfgcolor(FB_COLOR_1);
                        }
                        break;
                case 0x02:
                        if(bold) {
                                FBfgcolor(FB_COLOR_A);
                        } else {
                                FBfgcolor(FB_COLOR_2);
                        }
                        break;
                case 0x03:
                        if(bold) {
                                FBfgcolor(FB_COLOR_B);
                        } else {
                                FBfgcolor(FB_COLOR_3);
                        }
                        break;
                case 0x04:
                        if(bold) {
                                FBfgcolor(FB_COLOR_C);
                        } else {
                                FBfgcolor(FB_COLOR_4);
                        }
                        break;
                case 0x05:
                        if(bold) {
                                FBfgcolor(FB_COLOR_D);
                        } else {
                                FBfgcolor(FB_COLOR_5);
                        }
                        break;
                case 0x06:
                        if(bold) {
                                FBfgcolor(FB_COLOR_E);
                        } else {
                                FBfgcolor(FB_COLOR_6);
                        }
                        break;
                case 0x07:
                        if(bold) {
                                FBfgcolor(FB_COLOR_F);
                        } else {
                                FBfgcolor(FB_COLOR_7);
                        }
                        break;
                default:
                        // huh??
                        break;
        }
}

size_t fbcon::write(uint8_t c) {
        if(!diag) {
                switch(c) {
                        case 0x00: // ^@ \0
                                break;
                        case 0x01: // ^A home
                                pX = 0;
                                pY = 0;
                                break;
                        case 0x02: // ^B bold on
                                bold = true;
                                setprcol();
                                break;
                        case 0x03: // ^C bold and inverse and flash off
                                bold = false;
                                FBinverse(false);
                                FBflash(false);
                                setprcol();
                                break;
                        case 0x04: // ^D bold off
                                bold = false;
                                setprcol();
                                break;
                        case 0x05: // ^E flash on
                                FBflash(true);
                                break;
                        case 0x06: // ^F flash off
                                FBflash(false);
                                break;
                        case 0x07: // ^G \a
                                if(alert) {
                                        alert->alert();
                                }
                                break;
                        case 0x08: // ^H \b -- note backspace != left
                                if(pX > 0) {
                                        pX--;
                                } else {
                                        pX = _sizeX - 1;
                                        if(pY > 0) {
                                                pY--;
                                        }
                                }
                                break;
                        case 0x09: // ^I \t TAB
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
                        case 0x0a: // ^J \n DOWN with advance
                                if(pY == _sizeY - 1) {
                                        scroll();
                                } else {
                                        pY++;
                                }
                                break;
                        case 0x0b: // ^K \v
                                break;
                        case 0x0c: // ^L \f CLEAR/HOME
                                pX = 0;
                                pY = 0;
                                FBclear(FGcolor, BGcolor);
                                break;
                        case 0x0d: // ^M \r
                                pX = 0;
                                break;
                        case 0x0e: // ^N 0 black
                                prcol = 0;
                                setprcol();
                                break;
                        case 0x0f: // ^O 1 red
                                prcol = 1;
                                setprcol();
                                break;
                        case 0x10: // ^P 2 green
                                prcol = 2;
                                setprcol();
                                break;
                        case 0x11: // ^Q 3 yellow
                                prcol = 3;
                                setprcol();
                                break;
                        case 0x12: // ^R 4 blue
                                prcol = 4;
                                setprcol();
                                break;
                        case 0x13: // ^S 5 violet
                                prcol = 5;
                                setprcol();
                                break;
                        case 0x14: // ^T 6 cyan
                                prcol = 6;
                                setprcol();
                                break;
                        case 0x15: // ^U 7 white
                                prcol = 7;
                                setprcol();
                                break;
                        case 0x16: // ^V RIGHT
                                if(pX < _sizeX - 1) {
                                        pX++;
                                }
                                break;
                        case 0x17: // ^W UP
                                if(pY > 0) {
                                        pY--;
                                }
                                break;
                        case 0x18: // ^X DOWN
                                if(pY < _sizeY - 1) {
                                        pY++;
                                }
                                break;
                        case 0x19: // ^Y cursor on
                                cursor(true);
                                break;
                        case 0x1a: // ^Z LEFT
                                if(pX > 0) {
                                        pX--;
                                }
                                break;
                        case 0x1b: // ^[
                                break;
                        case 0x1c: // ^BACKSLASH
                                break;
                        case 0x1d: // ^] cursor off
                                cursor(false);
                                break;
                        case 0x1e: // ^^ inverse on
                                FBinverse(true);
                                break;
                        case 0x1f: // ^_ inverse off
                                FBinverse(false);
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
                prcol = 7;
                bold = false;
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
                FGcolor.color = FB_COLOR_7.color;
                BGcolor.color = FB_COLOR_0.color;

                // init frame buffers;
                charcell_t *p = FBcon;
                charcell_t *q = FBcpy;
                for(int i = 0; i < _sizeB; i++, p++, q++) {
                        p->cell = 0x007FU | FB_COLOR_F.color._color << 10;
                        q->cell = 0x0001U | FB_COLOR_F.color._color << 10;
                }
                it.priority(224); // use a modest priority, allow more important events to happen
                it.begin(this, 500000UL);
                updateFB();
                FBclear(FB_COLOR_7, FB_COLOR_0); // clear display
                initted = true;
        }
}

// RGB222 -> monochrome

uint8_t FB_to_monochrome(rgb222_t color) {
        // NOTE: the "color" is reversed on back-lit/reflective LCD,
        //       but is correct for LED displays.
        return (color.color._color) ? 1 : 0;
}

// RGB222 -> RGB565 LUTs
static const uint16_t FB_lt222_xx5[4] = {0x0000U, 0x0010U, 0x0018U, 0x001FU}; // B
static const uint16_t FB_lt222_x6x[4] = {0x0000U, 0x0400U, 0x0600U, 0x07C0U}; // G
static const uint16_t FB_lt222_5xx[4] = {0x0000U, 0x8000U, 0xC000U, 0xF800U}; // R
// RGB222 -> RGB565

uint16_t FB_to_565(rgb222_t color) {
        return (FB_lt222_5xx[color.color.r] | FB_lt222_x6x[color.color.g] | FB_lt222_xx5[color.color.b]);
}
