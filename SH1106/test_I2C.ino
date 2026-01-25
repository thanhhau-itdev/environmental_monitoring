#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Starting I2C Scanner...");

  Wire.begin(25, 26);
}

void loop() {
  byte error, address;
  int deviceCount = 0;

  Serial.println("Scanning I2C bus...");
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      deviceCount++;
    }
  }
  if (deviceCount == 0) {
    Serial.println("No I2C devices found.");
  }
  delay(5000);
}