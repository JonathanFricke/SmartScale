#include "main.h"
#include "smartScale.h"
#include <Arduino.h>

SmartScale scale;

void setup() { scale.init(); }

void loop() {
  Serial.begin(9600);
  scale.step();
}
