#include "EC_Sensor.h"

#define VREF    3300
#define ADC_RES 4095

EC_Sensor::EC_Sensor(uint8_t pin)
{
  _pin = pin;
  _voltage = 0;
}

void EC_Sensor::begin()
{
  _ec.begin();
  Serial.println(_ec._kvalueLow);
  Serial.println(_ec._kvalueHigh);
}

void EC_Sensor::update(EC_Data &data, float temperature)
{
  _voltage = (uint32_t)VREF * analogRead(_pin) / ADC_RES;
  data.voltage = _voltage;
  data.value   = _ec.readEC(_voltage, temperature);
}

void EC_Sensor::calibration(float temperature)
{
  _ec.calibration(_voltage, temperature);
}

void EC_Sensor::calibration(float temperature, char *cmd)
{
  _ec.calibration(_voltage, temperature, cmd);
}
