#include "LM35_Sensor.h"

#define VREF 3300
#define RES 4095
#define SAMPLE_COUNT 50

LM35_Sensor::LM35_Sensor(uint8_t pin)
{
  _pin = pin;
}

uint8_t LM35_Sensor::readTemperature()
{
  uint32_t sum = 0;

  for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
  {
    sum += analogRead(_pin);
    delay(2);
  }

  float voltage = (float)sum / SAMPLE_COUNT * VREF / RES;
  return (uint8_t)(voltage / 10.0);
}