#define MONOCHROME 0
#define USESPI 0
#define EIGHTBITS 0
#define EIGHTBITS16BITSIGNALS 0


#include <Arduino.h>
#include <fbcon.h>


#if USESPI
#if MONOCHROME
#include <SPI.h>
#include <U8g2lib.h>
U8G2_ST7565_ERC12864_ALT_F_4W_HW_SPI u8g2(U8G2_R0, 15, 4, 5);
#else

#include <SPI.h>
#include <ILI9341_t3.h>
const uint8_t TFT_DC = 9;
const uint8_t TFT_CS = 10;
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);
#endif
#else
// parallel
#include <ILI9341-816-bit_teensy.h>
#if EIGHTBITS
#if EIGHTBITS16BITSIGNALS
ILI9341_TFT tft(false, TFT_RST_16, TFT_BACKLIGHT_16);
#else
ILI9341_TFT tft(false);
#endif
#else
ILI9341_TFT tft(true);
#endif
#endif

class pixel : public fbcon_pixel {

        void set(uint16_t x, uint16_t y, rgb222_t color) {
#if MONOCHROME
                u8g2.setDrawColor(FB_to_monochrome(color));
                u8g2.drawPixel(x, y);
#else
                tft.drawPixel(x, y, FB_to_565(color));
#endif
        }

        void clear(uint16_t x, uint16_t y, rgb222_t color) {
#if MONOCHROME
                u8g2.setDrawColor(FB_to_monochrome(color));
                u8g2.drawPixel(x, y);
#else
                tft.drawPixel(x, y, FB_to_565(color));
#endif
        }

        void commit(void) {
#if MONOCHROME
                u8g2.sendBuffer();
#endif
        }

};

fbcon console = fbcon();
pixel _pixel = pixel();
uint16_t dispx;
uint16_t dispy;

void zzz(uint16_t p, uint16_t w, uint16_t q) {
        for(uint16_t i = p; i < p + w; i++) {
                console.FBset((i - p) * 2, q, i);
        }
        console.updateFB();
}

void console_setup(void) {
#if USESPI
#if MONOCHROME
        u8g2.setBusClock(30000000);
        u8g2.begin();
        u8g2.clear();
        console.begin(u8g2.getDisplayWidth(), u8g2.getDisplayHeight(), &_pixel);
#else
        tft.begin();
        tft.setClock(30000000);
        tft.setRotation(3);
        tft.fillScreen(ILI9341_GREEN);
        console.begin(tft.width(), tft.height(), &_pixel); // x and y are swapped because of rotation
#endif
#else
        tft.begin();
        tft.setRotation(3);
        tft.fillScreen(ILI9341_GREEN);
        console.begin(tft.width(), tft.height(), &_pixel); // x and y are swapped because of rotation
#endif

}

void setup() {
        while(!Serial) {
                yield();
        }
        Serial.begin(115200);
        console_setup();

        dispx = console.width();
        dispy = console.height();

        Serial.println(dispx);
        Serial.println(dispy);

        uint16_t wide = (dispx / 2) & 0x1ff0U;
        uint16_t tall = (dispy / 2);
        uint16_t sweep = wide * tall;
        while(1) {

                for(uint16_t i = 0; i < 0x03ffU; i += sweep) {
                        console.FBclear();
                        for(uint16_t j = 0; j < tall; j++) {
                                zzz(i + (wide * j), wide, j * 2);
                        }
                        delay(2000);
                }
        }
}

void loop() {
}
