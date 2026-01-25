#ifndef LM35_SENSOR_H
#define LM35_SENSOR_H

#include <Arduino.h>

class LM35_Sensor {
  public:
    explicit LM35_Sensor(uint8_t pin);
    uint8_t readTemperature();

  private:
    uint8_t _pin;
};

#endif
