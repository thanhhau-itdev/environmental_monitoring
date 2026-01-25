#include "PH_Sensor.h"

#define VREF    3300
#define RES 4095

PH_Sensor::PH_Sensor(uint8_t pin)
{
  _pin = pin;
  _voltage = 0;
}

void PH_Sensor::begin()
{
  _ph.begin();
}

void PH_Sensor::update(PH_Data &data, uint8_t temperature)
{
  _voltage = (uint32_t) VREF * analogRead(_pin) / RES;
  data.voltage = _voltage;
  data.value   = _ph.readPH(_voltage, temperature);
}

void PH_Sensor::calibration(uint8_t temperature)
{
  _ph.calibration(_voltage, temperature);
}
