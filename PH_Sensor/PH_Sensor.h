#ifndef PH_SENSOR_H
#define PH_SENSOR_H

#include <Arduino.h>
#include <EEPROM.h>
#include "DFRobot_ESP_PH.h"

class PH_Sensor {
public:
  struct PH_Data {
    uint16_t voltage;   // mV
    float    value;     // pH
  };

  explicit PH_Sensor(uint8_t pin);

  void begin();
  void update(PH_Data &data, float temperature);
  void calibration(float temperature);

private:
  uint8_t _pin;
  uint16_t _voltage;   // mV
  DFRobot_ESP_PH _ph;
};

#endif // PH_SENSOR_H