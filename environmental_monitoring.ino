#include <Arduino.h>
#include "LM35_Sensor.h"
#include "PH_Sensor.h"
#include "EC_Sensor.h"
#include "DO_Sensor.h"  
#include "pins_config.h"

LM35_Sensor lm35(TEMP_PIN);
DO_Sensor doSensor(DO_PIN);
PH_Sensor phSensor(PH_PIN);
EC_Sensor ecSensor(EC_PIN);

DO_Sensor::DO_Data doData;
PH_Sensor::PH_Data phData;
EC_Sensor::EC_Data ecData;

unsigned long lastTime_TEMP = 0, lastTime_DO = 0, 
              lastTime_PH = 0, lastTime_EC = 0, 
              lastTime_Serial = 0, lastTime_SendData = 0;

const unsigned long interval_TEMP = 1000, interval_DO = 1000, 
                    interval_PH = 1000, interval_EC = 1000, 
                    interval_Serial = 1000, interval_SendData = 5000;

uint8_t temp = 0;
uint16_t DO_value = 0;
float PH_value = 0, EC_value = 0;

void SendData()
{
  String query = "";
  query += String(temp) + ",";
  query += String(PH_value) + ",";
  query += String(EC_value) + ",";
  query += String(DO_value);

  Serial1.println(query);
}

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  
  ecSensor.begin();
  phSensor.begin();
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

    DO_value = doData.value;
  }

  if (now - lastTime_PH >= interval_PH) {
    lastTime_PH = now;
    phSensor.update(phData, temp);
    PH_value = phData.value;
  }

  if (now - lastTime_EC >= interval_EC) {
    lastTime_EC = now;
    ecSensor.update(ecData, temp);

    EC_value = ecData.value;
  }

  if (now - lastTime_Serial >= interval_Serial) {
    lastTime_Serial = now;
    
    Serial.print("Temp: "); Serial.print(temp);
    Serial.print(" DO: "); Serial.print(DO_value);
    Serial.print(" PH: "); Serial.print(PH_value, 2);
    Serial.print(" EC: "); Serial.println(EC_value, 2);
  }

  if (now - lastTime_SendData >= interval_SendData) {
    lastTime_SendData = now;
    SendData();
  }

  phSensor.calibration(temp);
  ecSensor.calibration(temp);
}

