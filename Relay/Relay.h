#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

class Relay {
  public:
    Relay(uint8_t pin, bool activeLevel = LOW);

    void begin();
    void on();
    void off();
    void toggle();
    bool state();

  private:
    uint8_t _pin;
    bool _activeLevel;
    bool _state;
};

#endif
