#include "DO_Sensor.h"

#define VREF   3300
#define ADC_RES 4095

#define CAL1_V 963
#define CAL1_T 26
#define CAL2_V 756
#define CAL2_T 19

const uint16_t DO_Sensor::DO_Table[41] = {
  14460,14220,13820,13440,13090,12740,12420,12110,11810,11530,
  11260,11010,10770,10530,10300,10080,9860,9660,9460,9270,
  9080,8900,8730,8570,8410,8250,8110,7960,7820,7690,
  7560,7430,7300,7180,7070,6950,6840,6730,6630,6530,6410
};

DO_Sensor::DO_Sensor(uint8_t doPin)
{
  _doPin = doPin;
}

uint16_t DO_Sensor::readDO(uint16_t voltage, uint8_t temperature)
{
  uint16_t saturation = (int16_t)(temperature - CAL2_T) * (CAL1_V - CAL2_V) / (CAL1_T - CAL2_T) + CAL2_V;
  return (uint32_t)voltage * DO_Table[temperature] / saturation;
}

void DO_Sensor::update(DO_Data &data, uint8_t temp)
{
  data.raw = analogRead(_doPin);
  data.voltage = (uint32_t)VREF * data.raw / ADC_RES;
  data.value = readDO(data.voltage, temp);
}
