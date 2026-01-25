#include <Arduino.h>
#include <Wire.h>
#include "LM35_Sensor/LM35_Sensor.h"
#include "DO_Sensor/DO_Sensor.h"
#include "EC_Sensor/EC_Sensor.h"
#include "DO_Sensor/DO_Sensor.h"
#include "SH1106/SH1106.h"
#include "Relay/Relay.h"
#include "pins_config.h"

Relay relay_TEMP(RELAY_TEMP_PIN);
Relay relay_DO(RELAY_DO_PIN);
Relay relay_PH(RELAY_PH_PIN);
Relay relay_EC(RELAY_EC_PIN);

LM35_Sensor lm35(TEMP_PIN);

DO_Sensor doSensor(DO_PIN);
PH_Sensor phSensor(PH_PIN);
EC_Sensor ecSensor(EC_PIN);

DO_Sensor::DO_Data doData;
PH_Sensor::PH_Data phData;
EC_Sensor::EC_Data ecData;

unsigned long lastTime_TEMP = 0, lastTime_DO = 0, 
              lastTime_PH = 0, lastTime_EC = 0, 
              lastTime_LCD = 0, lastTime_Serial = 0;

const unsigned long interval_TEMP = 1000, interval_DO = 1000, 
                    interval_PH = 1000, interval_EC = 1000, 
                    interval_LCD = 1000, interval_Serial = 1000;

uint8_t temp = 0;
uint16_t PH_voltage = 0, EC_voltage = 0, DO_voltage = 0, DO_value = 0;
float PH_value = 0, EC_value = 0;

void setup()
{
  Serial.begin(115200);

  relay_TEMP.begin();
  relay_DO.begin();
  relay_PH.begin();
  relay_EC.begin();

  ecSensor.begin();
  phSensor.begin();

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!oled.begin(0x3C)) {
    Serial.println(F("OLED init failed!"))
    while (1);
  }
  oled.clear();
  
  relay_TEMP.on();
  relay_DO.on();
  relay_PH.on();
  relay_EC.on();
}

void loop()
{
  unsigned long now = millis();

  if (now - lastTime_TEMP >= interval_TEMP) {
    lastTime_TEMP = now;
    temp = lm35.readTemperature();
  }

  if (now - lastTime_DO >= interval_DO) {
    lastTime_DO = now;
    doSensor.update(doData, temp);

    DO_voltage = doSensor.voltage;
    DO_value = doSensor.value;
  }

  if (now - lastTime_PH >= interval_PH) {
    lastTime_PH = now;
    phSensor.update(phData, temperature);

    PH_voltage = phSensor.voltage;
    PH_value = phSensor.value;
  }

  if (now - lastTime_EC >= interval_EC) {
    lastTime_EC = now;
    ecSensor.update(ecData, temp);

    EC_voltage = ecSensor.voltage;
    EC_value = ecSensor.value;
  }

  if (now - lastTime_Serial >= interval_Serial) {
    lastTime_Serial = now;
    
    Serial.print("DO mV: "); Serial.print(DO_voltage); Serial.print(" | ");
    Serial.print("DO value: "); Serial.println(DO_value);
    Serial.print("PH mV: "); Serial.print(PH_voltage); Serial.print(" | ");
    Serial.print("PH value: "); Serial.println(PH_value, 2);
    Serial.print("EC mV: "); Serial.print(EC_voltage); Serial.print(" | ");
    Serial.print("EC value: "); Serial.println(EC_value, 2);
  }

  if (now - lastTime_LCD >= interval_LCD) {
    lastTime_LCD = now;
    
    oled.showText(1, 12, 8, "Temp: " + String(temp));
    oled.showText(1, 12, 16, "DO: " + String(DO_value));
    oled.showText(1, 12, 24, "PH: " + String(PH_value));
    oled.showText(1, 12, 32, "EC: " + String(EC_value));
  }

  phSensor.calibration(temperature);
  ecSensor.calibration(temp);
}

