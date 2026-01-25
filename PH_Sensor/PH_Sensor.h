#ifndef PH_SENSOR_H
#define PH_SENSOR_H

#include <Arduino.h>
#include "DFRobot_PH.h"

class PH_Sensor {
  public:
    struct PH_Data {
      uint16_t voltage;   // mV
      float    value;     // pH
    };

    PH_Sensor(uint8_t pin);
    void begin();
    void update(PH_Data &data, uint8_t temperature);
    void calibration(uint8_t temperature);

  private:
    uint8_t _pin;
    uint16_t _voltage;
    DFRobot_PH _ph;
};

#endif
