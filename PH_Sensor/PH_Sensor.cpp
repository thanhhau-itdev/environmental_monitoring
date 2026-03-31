#include "PH_Sensor.h"

#define ESP_VREF     3300.0f
#define ESP_ADC_RES  4095.0f

PH_Sensor::PH_Sensor(uint8_t pin)
: _pin(pin), _voltage(0)
{
}

void PH_Sensor::begin()
{
  EEPROM.begin(32);
  _ph.begin();
}

void PH_Sensor::update(PH_Data &data, float temperature)
{
  uint16_t raw = analogRead(_pin);
  _voltage = (uint16_t)(raw * ESP_VREF / ESP_ADC_RES);

  data.voltage = _voltage;
  data.value   = _ph.readPH((float)_voltage, temperature);
}

void PH_Sensor::calibration(float temperature)
{
  _ph.calibration((float)_voltage, temperature);
}