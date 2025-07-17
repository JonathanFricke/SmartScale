#include "confirmState.h"
#include "config.h"
#include "measureState.h"
#include "smartScale.h"
#include "uploadState.h"

#include <Arduino.h>

ConfirmState::ConfirmState() {
  counter = 0;
  millisTimestamp = 0;
}

ConfirmState &ConfirmState::getInstance() {
  static ConfirmState instance;
  return instance;
}

void ConfirmState::init(SmartScale &scale) {
  counter = 0;
  millisTimestamp = 0;
  init_weight = scale.getWeight();
}

void ConfirmState::step(SmartScale &scale) {
  Serial.println("ConfirmState");
  scale.updateWeight();
  int measurement = scale.getWeight();

  if (abs(init_weight - measurement) > READING_TOLERANCE)
    changeState(scale, MeasureState::getInstance());

  bool animFinished = scale.renderer->animConfirm();
  if (animFinished) {
    if (millisTimestamp == 0)
      millisTimestamp = millis();
    else if (millis() - millisTimestamp >= WEIGHT_CONFIRM_DURATION)
      changeState(scale, UploadState::getInstance());
  }
}

void ConfirmState::changeState(SmartScale &scale, State &newState) {
  scale.renderer->reset();
  reset();
  newState.init(scale);
  scale.setState(&newState);
}
