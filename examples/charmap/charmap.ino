#include <Arduino.h>

#define MONOCHROME 0

#include <fbcon.h>
#if MONOCHROME
#include <SPI.h>
#include <U8g2lib.h>
U8G2_ST7565_ERC12864_ALT_F_4W_HW_SPI u8g2(U8G2_R0, 15, 5, 4);
#else
#include <SPI.h>
#include <ILI9341_t3.h>
const uint8_t TFT_DC = 9;
const uint8_t TFT_CS = 10;
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);
#endif

class pixel : public fbcon_pixel {
        // note, color is RGB565

        void set(uint16_t x, uint16_t y, rgb2_t color) {
#if MONOCHROME
                u8g2.setDrawColor(FB_to_monochrome(color));
                u8g2.drawPixel(x, y);
#else
                tft.drawPixel(x, y, FB_to_565(color));
#endif
        }

        void clear(uint16_t x, uint16_t y, rgb2_t color) {
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

void setup() {
        while(!Serial) {
                yield();
        }
        Serial.begin(115200);

#if MONOCHROME
        u8g2.setBusClock(30000000);
        u8g2.begin();
        u8g2.clear_total();
        console.begin(u8g2.width(), u8g2.height(), &pixel);
#else
        // init TFT. Need to get parallel going, as blitting on SPI is horribly slow.
        tft.begin();
        tft.setClock(30000000);
        tft.setRotation(3);
        tft.fillScreen(ILI9341_GREEN);
        console.begin(tft.width(), tft.height(), &_pixel); // x and y are swapped because of rotation
#endif
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
