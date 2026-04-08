#ifndef DO_SENSOR_H
#define DO_SENSOR_H

#include <Arduino.h>

class DO_Sensor {
  public:
    struct DO_Data {
      uint16_t raw;
      uint16_t voltage;
      float value;
    };

    DO_Sensor(uint8_t doPin);
    void update(DO_Data &data, uint8_t temp); 

  private:
    uint8_t _doPin;
    static const uint16_t DO_Table[41];
    uint16_t readDO(uint16_t voltage, uint8_t temperature);
};

#endif