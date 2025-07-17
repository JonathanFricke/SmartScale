#include "uploadState.h"
#include "config.h"
#include "measureState.h"
#include "secret.h"
#include "smartScale.h"

#include <Arduino.h>

UploadState::UploadState() { counter = 0; }

UploadState &UploadState::getInstance() {
  static UploadState instance;
  return instance;
}

void UploadState::reset() {
  counter = 0;
  millisTimestamp = 0;
  InternetClient::reset();
};

void UploadState::step(SmartScale &scale) {
  //   Serial.println("UploadState");
  bool blinkDone = scale.renderer->animBlink();
  if (!blinkDone)
    return;

  scale.internetClient->connectWifi(SSID, WIFI_PASSWORD);
  scale.internetClient->postWeight(scale.getWeight());
  if (InternetClient::postFinished) {
    if (InternetClient::postSuccess) {
      scale.renderer->renderCheckmark();
    } else {
      scale.renderer->renderCross();
    }

    if (millis() - millisTimestamp >= POST_RESULT_DURATION)
      changeState(scale, MeasureState::getInstance()); // TODO: Change state to compare previous state
  } else {
    scale.renderer->animLoading();
    millisTimestamp = millis();
  }
}

void UploadState::changeState(SmartScale &scale, State &newState) {
  scale.renderer->reset();
  reset();
  newState.init(scale);
  scale.setState(&newState);
}
