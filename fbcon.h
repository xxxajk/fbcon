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

// 500000 microseconds is 0.5 seconds
#define FB_BLINK_RATE (500000UL)

typedef struct {

        union {

                struct {
                        uint8_t r : 2;
                        uint8_t g : 2;
                        uint8_t b : 2;
                } __attribute__ ((packed));
                uint8_t color : 8;
        };
} rgb2_t;

/* RGB222 colors, 64 colors total */
const rgb2_t FB_R0 = {
        {.r = 0U}
};
const rgb2_t FB_R1 = {
        {.r = 1U}
};
const rgb2_t FB_R2 = {
        {.r = 2U}
};
const rgb2_t FB_R3 = {
        {.r = 3U}
};
const rgb2_t FB_G0 = {
        {.g = 0U}
};
const rgb2_t FB_G1 = {
        {.g = 1U}
};
const rgb2_t FB_G2 = {
        {.g = 2U}
};
const rgb2_t FB_G3 = {
        {.g = 3U}
};
const rgb2_t FB_B0 = {
        {.b = 0U}
};
const rgb2_t FB_B1 = {
        {.b = 1U}
};
const rgb2_t FB_B2 = {
        {.b = 2U}
};
const rgb2_t FB_B3 = {
        {.b = 3U}
};

#define FB_MK_COLOR(name, rr, gg, bb) const rgb2_t name __attribute__((used)) = {{.color=(uint8_t)(rr.color|gg.color|bb.color)}};

// Common named Xterm colors.
// TO-DO: only defined 16, there's actually 64 available
FB_MK_COLOR(FB_BLACK, FB_R0, FB_G0, FB_B0)
FB_MK_COLOR(FB_MAROON, FB_R1, FB_G0, FB_B0)
FB_MK_COLOR(FB_GREEN, FB_R0, FB_G1, FB_B0)
FB_MK_COLOR(FB_OLIVE, FB_R1, FB_G1, FB_B0)
FB_MK_COLOR(FB_NAVY, FB_R0, FB_G0, FB_B1)
FB_MK_COLOR(FB_PURPLE, FB_R1, FB_G0, FB_B1)
FB_MK_COLOR(FB_TEAL, FB_R0, FB_G1, FB_B1)
FB_MK_COLOR(FB_SILVER, FB_R2, FB_G2, FB_B2)
FB_MK_COLOR(FB_GRAY, FB_R1, FB_G1, FB_B1)
FB_MK_COLOR(FB_RED, FB_R3, FB_G0, FB_B0)
FB_MK_COLOR(FB_LIME, FB_R0, FB_G3, FB_B0)
FB_MK_COLOR(FB_YELLOW, FB_R3, FB_G3, FB_B0)
FB_MK_COLOR(FB_BLUE, FB_R0, FB_G0, FB_B3)
FB_MK_COLOR(FB_FUCHSIA, FB_R3, FB_G0, FB_B3)
FB_MK_COLOR(FB_AQUA, FB_R0, FB_G3, FB_B3)
FB_MK_COLOR(FB_WHITE, FB_R3, FB_G3, FB_B3)

typedef struct {

        union {
                uint16_t cell;

                struct {
                        uint8_t c : 8;
                        bool inverse : 1;
                        bool flash : 1;
                        uint8_t color : 6; // 6bit packed RRGGBB, 64 colors
                } __attribute__ ((packed));
        };
} charcell_t;

class fbcon_pixel {
public:

        fbcon_pixel() {
        };
        virtual void set(uint16_t x, uint16_t y, rgb2_t color);
        virtual void clear(uint16_t x, uint16_t y, rgb2_t color);
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
        rgb2_t BGcolor; // RGB222
        rgb2_t FGcolor; // RGB222
        bool flash;
        bool state;
        volatile bool user;
        volatile uint16_t _posX;
        volatile uint16_t _posY;
        volatile uint16_t pX;
        volatile uint16_t pY;
        bool inv;
        bool fla;
        bool diag;
        bool initted;
        fbcon_pixel *pixel;
        fbcon_alert *alert;

        void drawbyte(uint16_t x, uint16_t y, uint8_t b, rgb2_t fg);
        void drawSymbol(uint16_t x, uint16_t y, charcell_t s);
        void CISR(void);
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

        void FBset(uint16_t x, uint16_t y, rgb2_t c, uint16_t s) {
                if(x >= _sizeX) return;
                if(y >= _sizeY) return;
                if(s > 1023) return;
                FGcolor.color = c.color;
                FBset(x, y, s);
        };

        void FBfgcolor(rgb2_t color) {
                FGcolor = color;
        };

        void FBbgcolor(rgb2_t color) {
                BGcolor = color;
                // invalidate bitmap
                                charcell_t *p = FBcon;
                charcell_t *q = FBcpy;
                for(int i = 0; i < _sizeB; i++, p++, q++) {
                        q->cell=~p->cell;
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
        void FBclear(rgb2_t color, rgb2_t bg);

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
extern uint8_t FB_to_monochrome(rgb2_t color);
extern uint16_t FB_to_565(rgb2_t color);

#endif /* FBCON_H */
