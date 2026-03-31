#include "LM35_Sensor.h"

#define VREF 3300
#define RES 4095
#define SAMPLE_COUNT 20

LM35_Sensor::LM35_Sensor(uint8_t pin)
{
  _pin = pin;
}

uint8_t LM35_Sensor::readTemperature()
{
  uint32_t sumTemp = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    sumTemp += (uint32_t)VREF * analogRead(_pin) / RES;
  }
  return (uint8_t)(sumTemp / 10);
}
