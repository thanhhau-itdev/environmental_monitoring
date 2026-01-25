#include "Relay/Relay.h"

Relay relay1(5);

void setup() {
  relay1.begin();
}

void loop() {
  relay1.on();
  delay(1000);
  relay1.off();
  delay(1000);
}
