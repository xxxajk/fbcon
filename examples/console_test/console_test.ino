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

void dispcolor(const char *text, rgb222_t color) {
        console.FBfgcolor(color);
        console.FBinverse(true);
        console.print(" ");
        console.FBinverse(false);
        console.FBfgcolor(FB_COLOR_7);
        console.print(" ");
        console.print(text);
        console.print("\r\n");
}


charcell_t foo;

void setup() {
        foo = 1U;

        while(!Serial) {
                yield();
        }
        Serial.begin(115200);

        console_setup();
        dispx = (console.width() / 16)*16;
        uint16_t chgh = console.height();
        dispy = (512 / dispx) < chgh ? (512 / dispx) : chgh;
        chgh--;
        Serial.println(dispx);
        Serial.println(dispy);
        // 0123456789ABCDEF
        //   Hello world!
        // reset clear screen, reset attributes, turn on cursor, set color to white
        console.print("\f\003\031\025");
        delay(1002);
        // bg color can't be set via printf
        console.FBbgcolor(FB_COLOR_5);
        for(uint8_t i = 0; i < chgh; i++) {
                console.print(i);
                console.println(" Manual scroll");
        }
        // print in bold green, inverse and reset attribs
        console.print("\020\002\036* Hello World! *\003\025");
        delay(501);
        console.FBbgcolor(FB_COLOR_0);
        for(uint8_t i = 0; i < chgh; i++) {
                delay(501);
                console.print("\n");
        }

        for(int i = 0; i < 8; i++) {
                console.print("\001\n");
                for(int j = 0; j < i; j++) {
                        console.print("\026");
                }
                console.print(" ");
                console.print("\017\002\005Looks good!\003");
                delay(501);
        }
        console.println();
#if !MONOCHROME
        // direct color setting
        dispcolor("FB_RED1", FB_RED1);
        dispcolor("FB_RED2", FB_RED2);
        dispcolor("FB_RED3", FB_RED3);
        dispcolor("FB_GREEN1", FB_GREEN1);
        dispcolor("FB_GREEN2", FB_GREEN2);
        dispcolor("FB_GREEN3", FB_GREEN3);
        dispcolor("FB_BLUE1", FB_BLUE1);
        dispcolor("FB_BLUE2", FB_BLUE2);
        dispcolor("FB_BLUE3", FB_BLUE3);
        // and via print
        console.println("\036\017 \025\003 FB_COLOR_1");
        console.println("\036\020 \025\003 FB_COLOR_2");
        console.println("\036\021 \025\003 FB_COLOR_3");
        console.println("\036\022 \025\003 FB_COLOR_4");
        console.println("\036\023 \025\003 FB_COLOR_5");
        console.println("\036\024 \025\003 FB_COLOR_6");
        console.println("\036\025 \025\003 FB_COLOR_7");

        console.println("\036\016\002 \025\003 FB_COLOR_8 (AKA BOLD 0)");
        console.println("\036\017\002 \025\003 FB_COLOR_9 (AKA BOLD 1)");
        console.println("\036\020\002 \025\003 FB_COLOR_A (AKA BOLD 2)");
        console.println("\036\021\002 \025\003 FB_COLOR_B (AKA BOLD 3)");
        console.println("\036\022\002 \025\003 FB_COLOR_C (AKA BOLD 4)");
        console.println("\036\023\002 \025\003 FB_COLOR_D (AKA BOLD 5)");
        console.println("\036\024\002 \025\003 FB_COLOR_E (AKA BOLD 6)");
        console.println("\036\025\002 \025\003 FB_COLOR_F (AKA BOLD 7)");
#endif
}

void loop() {
}
