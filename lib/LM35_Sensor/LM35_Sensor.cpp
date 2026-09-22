#include "LM35_Sensor.h"

#define VREF 3300.0
#define RES 4095.0

LM35_Sensor::LM35_Sensor(uint8_t pin)
{
  _pin = pin;
}

uint8_t LM35_Sensor::readTemperature()
{
  return (analogRead(_pin) * VREF / RES) / 10.0;
}