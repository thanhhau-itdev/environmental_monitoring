#ifndef EC_SENSOR_H
#define EC_SENSOR_H

#include <Arduino.h>
#include "DFRobot_EC.h"

class EC_Sensor {
  public:
    struct EC_Data {
      uint16_t voltage;
      float    value;
    };

    EC_Sensor(uint8_t pin);
    void begin();
    void update(EC_Data &data, uint8_t temperature);
    void calibration(uint8_t temperature);

  private:
    uint8_t _pin;
    uint16_t _voltage;
    DFRobot_EC _ec;
};

#endif
