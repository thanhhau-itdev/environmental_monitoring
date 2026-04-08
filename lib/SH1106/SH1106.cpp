#include "SH1106.h"

SH1106::SH1106() : display(128, 64, &Wire, -1) {}

bool SH1106::begin(uint8_t addr) {
    if (!display.begin(addr, true)) {
        return false;
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println(F("Ready!"));
    display.display();
    delay(1000);
    return true;
}

void SH1106::clear() {
    display.clearDisplay();
}

void SH1106::showText(int size, int x, int y, const String &text) {
    display.setTextSize(size);
    display.setCursor(x, y);
    display.print(text);
}

void SH1106::showValue(int size, int x, int y, const String &label, const String &value, const String &unit) {
    display.setTextSize(size);
    display.setCursor(x, y);
    display.print(label);
    display.print(value);
    if (unit != "") display.print(unit);
}

void SH1106::refresh() {
    display.display();
}
