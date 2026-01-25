#ifndef SH1106_H
#define SH1106_H

#include <Arduino.h>
#include <Adafruit_SH110X.h>

class SH1106 {
    public:
        SH1106();
        bool begin(uint8_t addr = 0x3C);

        void clear();
        void showText(int size, int x, int y, const String &text);
        void showValue(int size, int x, int y, const String &label, const String &value, const String &unit = "");
        void refresh();

    private:
        Adafruit_SH1106G display;
};

#endif
