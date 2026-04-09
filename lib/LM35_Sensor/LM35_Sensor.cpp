#include "LM35_Sensor.h"

#define VREF 3300
#define RES 4095

LM35_Sensor::LM35_Sensor(uint8_t pin)
{
  _pin = pin;
}

uint8_t LM35_Sensor::readTemperature()
{
  return (uint8_t)(((uint32_t)VREF * analogRead(_pin) / RES) / 10);
}

