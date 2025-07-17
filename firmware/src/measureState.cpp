#include "measureState.h"
#include "config.h"
#include "confirmState.h"
#include "smartScale.h"

#include <Arduino.h>

// #include "nextPossibleState.h"

MeasureState::MeasureState() {
  counter = 0;
  millisTimestamp = 0;
}

MeasureState &MeasureState::getInstance() {
  static MeasureState instance;
  return instance;
}

void MeasureState::step(SmartScale &scale) {
  int prev_measurement = scale.getWeight();
  scale.updateWeight();
  int measurement = scale.getWeight();
  scale.renderer->animWeight(measurement);

  Serial.print("MeasureState: ");
  Serial.print(measurement);
  Serial.println("");

  if (abs(prev_measurement - measurement) < READING_TOLERANCE && measurement >= MINIMAL_WEIGHT) {
    if (millisTimestamp == 0)
      millisTimestamp = millis();
  } else {
    reset();
  }

  if (millisTimestamp != 0 && millis() - millisTimestamp >= WEIGHT_STABILIZING_DURATION) {
    changeState(scale, ConfirmState::getInstance());
  }
}

void MeasureState::changeState(SmartScale &scale, State &newState) {
  scale.renderer->reset();
  reset();
  newState.init(scale);
  scale.setState(&newState);
}

// void MeasureState::reset() { counter = 0; }
