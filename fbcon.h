/*
 * File:   fbcon.h
 * Author: xxxajk
 *
 * Created on September 12, 2021, 4:11 AM
 */

#ifndef FBCON_H
#define FBCON_H
#include <Arduino.h>
#include <limits.h>
#include <IntervalClassTimer2.h>
#include "CODERMONO.h"

/* Screen controls:
 *  DEC | C  |CTRL| HEX  | OCT
 * [  7 | \a | ^G | 0x07 | 007 ] ring bell (Call alert sound method, if available)
 * [ 25 |    | ^Y | 0x19 | 031 ] cursor on
 * [ 29 |    | ^] | 0x1D | 035 ] cursor off
 *
 * [  8 | \b | ^H | 0x08 | 010 ] backspace, does not perform rubout
 * [  9 | \t | ^I | 0x09 | 011 ] standard 8 space aligned tab
 * [ 10 | \n | ^J | 0x0A | 012 ] new line, down one line without carriage return, scroll if necessary
 * [ 11 | \v | ^K | 0x0B | 013 ] vertical tab, ignored
 * [ 12 | \f | ^L | 0x0C | 014 ] form feed, clear screen, home cursor
 * [ 13 | \r | ^M | 0x0D | 015 ] carriage return return cursor to beginning of current line.
 * [  1 |    | ^A | 0x01 | 001 ] home cursor
 * [ 23 |    | ^W | 0x17 | 027 ] cursor up, won't cause screen to scroll in reverse
 * [ 24 |    | ^X | 0x18 | 030 ] cursor down, won't cause screen to scroll
 * [ 26 |    | ^Z | 0x1A | 032 ] cursor left, won't wrap to previous line
 * [ 22 |    | ^V | 0x16 | 026 ] cursor right, won't wrap to next line
 *
 * [  3 |    | ^C | 0x03 | 003 ] normal text bold inverse and flash off
 * [  2 |    | ^B | 0x02 | 002 ] bold on
 * [  4 |    | ^D | 0X04 | 004 ] bold off
 * [  5 |    | ^E | 0x05 | 005 ] flash on
 * [  6 |    | ^F | 0x06 | 006 ] flash off
 * [ 30 |    | ^^ | 0x1e | 036 ] inverse on
 * [ 31 |    | ^_ | 0x1f | 037 ] inverse off
 * [ 14 |    | ^N | 0x0E | 016 ] black
 * [ 15 |    | ^O | 0x0F | 017 ] dark red
 * [ 16 |    | ^P | 0x10 | 020 ] dark green
 * [ 17 |    | ^Q | 0x11 | 021 ] dark yellow
 * [ 18 |    | ^R | 0x12 | 022 ] dark blue
 * [ 19 |    | ^S | 0x13 | 023 ] dark violet
 * [ 20 |    | ^T | 0x14 | 024 ] dark cyan
 * [ 21 |    | ^U | 0x15 | 025 ] dark white
 *
 *
 * 0x27 (standard escape) is currently reserved and ignored.
 *
 */

// 500000 microseconds is 0.5 seconds
#define FB_BLINK_RATE (500000UL)

union rgb222 {
        uint8_t _color;
        struct {
                uint8_t r : 2;
                uint8_t g : 2;
                uint8_t b : 2;
        } __attribute__ ((packed));

/*
        rgb2& operator=(const rgb2& other) {
                _color = other._color;
                return *this;
        }
        rgb2& operator=(rgb2& other) {
                _color = other._color;
                return *this;
        }
*/
        rgb222& operator=(const uint8_t& other) {
                _color = other;
                return *this;
        }

/*
        // |=, |
        const rgb2& operator|(const rgb2& other) {
                _color |= other._color;
                return *this;
        }
*/
        //rgb2& operator|(rgb2& other) {
        //        color |= other.color;
        //        return *this;
        //}
        //rgb2 operator|(const unsigned int other) {
        //        return (rgb2) (color | other);
        //}
};

// wrap union to get an alias type
typedef struct {
        rgb222 color;
} rgb222_t;

/* RGB222 colors, 64 colors total */
const rgb222_t FB_R0 = {
        {.r = 0U}
};
const rgb222_t FB_R1 = {
        {.r = 1U}
};
const rgb222_t FB_R2 = {
        {.r = 2U}
};
const rgb222_t FB_R3 = {
        {.r = 3U}
};
const rgb222_t FB_G0 = {
        {.g = 0U}
};
const rgb222_t FB_G1 = {
        {.g = 1U}
};
const rgb222_t FB_G2 = {
        {.g = 2U}
};
const rgb222_t FB_G3 = {
        {.g = 3U}
};
const rgb222_t FB_B0 = {
        {.b = 0U}
};
const rgb222_t FB_B1 = {
        {.b = 1U}
};
const rgb222_t FB_B2 = {
        {.b = 2U}
};
const rgb222_t FB_B3 = {
        {.b = 3U}
};

#define FB_MK_COLOR(name, rr, gg, bb) const rgb222_t name __attribute__((used)) = {{._color=(uint8_t)(rr.color._color|gg.color._color|bb.color._color)}};

// Primary console colors
FB_MK_COLOR(FB_COLOR_0, FB_R0, FB_G0, FB_B0)
FB_MK_COLOR(FB_COLOR_1, FB_R2, FB_G0, FB_B0)
FB_MK_COLOR(FB_COLOR_2, FB_R0, FB_G2, FB_B0)
FB_MK_COLOR(FB_COLOR_3, FB_R2, FB_G2, FB_B0)
FB_MK_COLOR(FB_COLOR_4, FB_R0, FB_G0, FB_B2)
FB_MK_COLOR(FB_COLOR_5, FB_R2, FB_G0, FB_B2)
FB_MK_COLOR(FB_COLOR_6, FB_R0, FB_G2, FB_B2)
FB_MK_COLOR(FB_COLOR_7, FB_R2, FB_G2, FB_B2)
// Bold primary console colors
FB_MK_COLOR(FB_COLOR_8, FB_R1, FB_G1, FB_B1)
FB_MK_COLOR(FB_COLOR_9, FB_R3, FB_G0, FB_B0)
FB_MK_COLOR(FB_COLOR_A, FB_R0, FB_G3, FB_B0)
FB_MK_COLOR(FB_COLOR_B, FB_R3, FB_G3, FB_B0)
FB_MK_COLOR(FB_COLOR_C, FB_R0, FB_G0, FB_B3)
FB_MK_COLOR(FB_COLOR_D, FB_R3, FB_G0, FB_B3)
FB_MK_COLOR(FB_COLOR_E, FB_R0, FB_G3, FB_B3)
FB_MK_COLOR(FB_COLOR_F, FB_R3, FB_G3, FB_B3)

#define FB_DARK_RED FB_COLOR_1
#define FB_DARK_GREEN FB_COLOR_2
#define FB_DARK_YELLOW FB_COLOR_3
#define FB_DARK_BLUE FB_COLOR_4
#define FB_DARK_VIOLET FB_COLOR_5
#define FB_DARK_CYAN FB_COLOR_6
#define FB_DARK_WHITE FB_COLOR_7



// Common named Xterm color names.
// TO-DO: only defined 16, there's actually 64 available
// These are also horribly dark, even on a real Xterminal

#define FB_XBLACK FB_COLOR_0
FB_MK_COLOR(FB_XMAROON, FB_R1, FB_G0, FB_B0)
FB_MK_COLOR(FB_XGREEN, FB_R0, FB_G1, FB_B0)
FB_MK_COLOR(FB_XOLIVE, FB_R1, FB_G1, FB_B0)
FB_MK_COLOR(FB_XNAVY, FB_R0, FB_G0, FB_B1)
FB_MK_COLOR(FB_XPURPLE, FB_R1, FB_G0, FB_B1)
FB_MK_COLOR(FB_XTEAL, FB_R0, FB_G1, FB_B1)
FB_MK_COLOR(FB_XSILVER, FB_R2, FB_G2, FB_B2)
FB_MK_COLOR(FB_XGRAY, FB_R1, FB_G1, FB_B1)
FB_MK_COLOR(FB_XRED, FB_R3, FB_G0, FB_B0)
FB_MK_COLOR(FB_XLIME, FB_R0, FB_G3, FB_B0)
FB_MK_COLOR(FB_XYELLOW, FB_R3, FB_G3, FB_B0)
FB_MK_COLOR(FB_XBLUE, FB_R0, FB_G0, FB_B3)
FB_MK_COLOR(FB_XFUCHSIA, FB_R3, FB_G0, FB_B3)
FB_MK_COLOR(FB_XAQUA, FB_R0, FB_G3, FB_B3)
FB_MK_COLOR(FB_XWHITE, FB_R3, FB_G3, FB_B3)

FB_MK_COLOR(FB_RED1, FB_R1, FB_G0, FB_B0)
FB_MK_COLOR(FB_RED2, FB_R2, FB_G0, FB_B0)
FB_MK_COLOR(FB_RED3, FB_R3, FB_G0, FB_B0)
FB_MK_COLOR(FB_GREEN1, FB_R0, FB_G1, FB_B0)
FB_MK_COLOR(FB_GREEN2, FB_R0, FB_G2, FB_B0)
FB_MK_COLOR(FB_GREEN3, FB_R0, FB_G3, FB_B0)
FB_MK_COLOR(FB_BLUE1, FB_R0, FB_G0, FB_B1)
FB_MK_COLOR(FB_BLUE2, FB_R0, FB_G0, FB_B2)
FB_MK_COLOR(FB_BLUE3, FB_R0, FB_G0, FB_B3)



union charcell_t {
        uint16_t cell;

        struct {
                uint8_t c : 8;
                bool inverse : 1;
                bool flash : 1;
                uint8_t color : 6; // 6bit packed RRGGBB, 64 colors
        } __attribute__ ((packed));


        // assignment =,

        charcell_t& operator=(const charcell_t& other) {
                cell = other.cell;
                return *this;
        }

        charcell_t& operator=(const uint16_t& other) {
                cell = other;
                return *this;
        }

        charcell_t& operator=(charcell_t& other) {
                cell = other.cell;
                return *this;
        }

        charcell_t& operator=(uint16_t& other) {
                cell = other;
                return *this;
        }
        // &=

        charcell_t& operator&=(const charcell_t& other) {
                cell &= other.cell;
                return *this;
        }

        charcell_t& operator&=(const uint16_t& other) {
                cell &= other;
                return *this;
        }

        charcell_t& operator&=(charcell_t& other) {
                cell &= other.cell;
                return *this;
        }

        charcell_t& operator&=(uint16_t& other) {
                cell &= other;
                return *this;
        }
        // ^=

        charcell_t& operator^=(const charcell_t& other) {
                cell ^= other.cell;
                return *this;
        }

        charcell_t& operator^=(const uint16_t& other) {
                cell ^= other;
                return *this;
        }

        charcell_t& operator^=(charcell_t& other) {
                cell ^= other.cell;
                return *this;
        }

        charcell_t& operator^=(uint16_t& other) {
                cell ^= other;
                return *this;
        }
        // |=

        charcell_t& operator|=(const charcell_t& other) {
                cell |= other.cell;
                return *this;
        }

        charcell_t& operator|=(const uint16_t& other) {
                cell |= other;
                return *this;
        }

        charcell_t& operator|=(charcell_t& other) {
                cell |= other.cell;
                return *this;
        }

        charcell_t& operator|=(uint16_t& other) {
                cell |= other;
                return *this;
        }

        // COLORS ONLY
        // +=

        charcell_t& operator+=(const uint8_t& other) {
                color += other;
                return *this;
        }
        charcell_t& operator+=(uint8_t& other) {
                color += other;
                return *this;
        }
        // -=

        charcell_t& operator-=(const uint8_t& other) {
                color -= other;
                return *this;
        }
        charcell_t& operator-=(uint8_t& other) {
                color -= other;
                return *this;
        }
        // *=

        charcell_t& operator*=(const uint8_t& other) {
                color *= other;
                return *this;
        }
        charcell_t& operator*=(uint8_t& other) {
                color *= other;
                return *this;
        }
        // /=

        charcell_t& operator/=(const uint8_t& other) {
                color /= other;
                return *this;
        }
        charcell_t& operator/=(uint8_t& other) {
                color /= other;
                return *this;
        }
        // %=

        charcell_t& operator%=(const uint8_t& other) {
                color %= other;
                return *this;
        }
        charcell_t& operator%=(uint8_t& other) {
                color %= other;
                return *this;
        }
        /*
        // color intensity shifting
        // >>=
        charcell_t& operator>>=(uint8_t& other) {
                color >>= other;
                return *this;
        }
        // <<=

        charcell_t& operator%=(uint8_t& other) {
                color <<= other;
                return *this;
        }
        */

        // logical

        bool operator==(const charcell_t& other) {
                return cell == other.cell;
        }

        bool operator==(const uint16_t& other) {
                return cell == other;
        }

        bool operator!=(const charcell_t& other) {
                return cell != other.cell;
        }

        bool operator!=(const uint16_t& other) {
                return cell != other;
        }

        bool operator>(const charcell_t& other) {
                return cell > other.cell;
        }

        bool operator>(const uint16_t& other) {
                return cell > other;
        }

        bool operator<(const charcell_t& other) {
                return cell < other.cell;
        }

        bool operator<(const uint16_t& other) {
                return cell < other;
        }

        bool operator>=(const charcell_t& other) {
                return cell >= other.cell;
        }

        bool operator>=(const uint16_t& other) {
                return cell >= other;
        }

        bool operator<=(const charcell_t& other) {
                return cell <= other.cell;
        }

        bool operator<=(const uint16_t& other) {
                return cell <= other;
        }

        bool operator||(const charcell_t& other) {
                return cell || other.cell;
        }

        bool operator||(const uint16_t& other) {
                return cell || other;
        }

        bool operator&&(const charcell_t& other) {
                return cell && other.cell;
        }

        bool operator&&(const uint16_t& other) {
                return cell && other;
        }

};

class fbcon_pixel {
public:

        fbcon_pixel() {
        };
        virtual void set(uint16_t x, uint16_t y, rgb222_t color);
        virtual void clear(uint16_t x, uint16_t y, rgb222_t color);
        virtual void commit(void);
};

class fbcon_alert {
public:

        fbcon_alert() {
        };
        virtual void alert(void);
};

class fbcon : public ictCISR, public Print {
private:
        IntervalClassTimer2 it;
        const uint8_t *symbits;
        charcell_t *FBcon;
        charcell_t *FBcpy;
        uint16_t _sizeB;
        uint16_t _sizeX;
        uint16_t _sizeY;
        rgb222_t BGcolor; // RGB222
        rgb222_t FGcolor; // RGB222
        bool flash;
        bool state;
        volatile bool user;
        volatile uint16_t _posX;
        volatile uint16_t _posY;
        volatile uint16_t pX;
        volatile uint16_t pY;
        uint8_t prcol;
        bool inv;
        bool fla;
        bool bold;
        bool diag;
        bool initted;
        fbcon_pixel *pixel;
        fbcon_alert *alert;

        void drawbyte(uint16_t x, uint16_t y, uint8_t b, rgb222_t fg);
        void drawSymbol(uint16_t x, uint16_t y, charcell_t s);
        void CISR(void);
        void setprcol(void);
public:

        fbcon() {
                alert = NULL;
                pixel = NULL;
                initted = false;
        };
        void begin(uint16_t x, uint16_t y, fbcon_pixel *_pixel, fbcon_alert *_alert);

        void begin(uint16_t x, uint16_t y, fbcon_pixel *_pixel) {
                begin(x, y, _pixel, NULL);
        };
        void updateFB(void);
        void scroll(void);
        void FBset(uint16_t x, uint16_t y, uint16_t s);

        void FBset(uint16_t x, uint16_t y, rgb222_t c, uint16_t s) {
                if(x >= _sizeX) return;
                if(y >= _sizeY) return;
                if(s > 1023) return;
                FGcolor.color = c.color;
                FBset(x, y, s);
        };

        void FBfgcolor(rgb222_t color) {
                FGcolor = color;
        };

        void FBbgcolor(rgb222_t color) {
                BGcolor = color;
                // invalidate bitmap
                charcell_t *p = FBcon;
                charcell_t *q = FBcpy;
                for(int i = 0; i < _sizeB; i++, p++, q++) {
                        q->cell = ~p->cell;
                }
        };

        void FBflash(bool on) {
                fla = on;
        };

        void FBinverse(bool on) {
                inv = on;
        };

        void FBdiag(bool on) {
                diag = on;
        };
        void FBclear(rgb222_t color, rgb222_t bg);

        void FBclear(void) {
                FBclear(FGcolor, BGcolor);
        };

        uint16_t width(void) {
                return _sizeX;
        };

        uint16_t height(void) {
                return _sizeY;
        };

        void cursor(bool on) {
                flash = on;
        };

        void pos(uint16_t *x, uint16_t *y, bool get) {
                if(get) {
                        *x = ((pX == _sizeX) ? _sizeX - 1 : pX);
                        *y = pY;
                } else {
                        pX = *x;
                        pY = *y;
                }
        };

        charcell_t FBget(uint16_t x, uint16_t y);

        virtual int availableForWrite(void) {
                return INT_MAX;
        };

        virtual void flush(void) {
                updateFB();
        };

        virtual size_t write(uint8_t c);

        inline size_t write(unsigned long n) {
                return write((uint8_t)n);
        };

        inline size_t write(long n) {
                return write((uint8_t)n);
        };

        inline size_t write(unsigned int n) {
                return write((uint8_t)n);
        };

        inline size_t write(int n) {
                return write((uint8_t)n);
        };
        using Print::write; // pull in write(str) and write(buf, size) from Print

        operator bool() {
                return initted;
        };
};

// color utilities
extern uint8_t FB_to_monochrome(rgb222_t color);
extern uint16_t FB_to_565(rgb222_t color);

#endif /* FBCON_H */
