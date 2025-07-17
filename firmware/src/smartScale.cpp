#include "smartScale.h"
#include "config.h"
#include "measureState.h"
#include "pins.h"

#include <Arduino.h>
#include <WiFi.h>

SmartScale::SmartScale() : loadcell(HX711_DOUT_PIN, HX711_SCK_PIN) {}

SmartScale::~SmartScale() {}

void SmartScale::init() {
  Serial.println("Init");
  setupLoadcell();
  renderer = &Renderer::getInstance();
  internetClient = &InternetClient::getInstance();
  state = &MeasureState::getInstance();
}

void SmartScale::step() { state->step(*this); }

void SmartScale::setupLoadcell() {
  Serial.println("Setup Loadcell");
  loadcell.powerUp();
  loadcell.begin();
  loadcell.start(LOADCELL_STABILIZING_TIME, false);
  loadcell.setSamplesInUse(LOADCELL_NUM_SAMPLES);

  if (loadcell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  } else {
    loadcell.setCalFactor(LOADCELL_CALIBRATION_VALUE); // set calibration factor (float)
    Serial.println("Startup is complete");
  }
  while (!loadcell.update())
    ;
  Serial.print("Calibration value: ");
  Serial.println(loadcell.getCalFactor());
  Serial.print("HX711 measured conversion time ms: ");
  Serial.println(loadcell.getConversionTime());
  Serial.print("HX711 measured sampling rate HZ: ");
  Serial.println(loadcell.getSPS());
  Serial.print("HX711 measured settlingtime ms: ");
  Serial.println(loadcell.getSettlingTime());
  Serial.println("Note that the settling time may increase significantly if "
                 "you use delay() in your sketch!");
  if (loadcell.getSPS() < 7) {
    Serial.println("!!Sampling rate is lower than specification, check "
                   "MCU>HX711 wiring and pin designations");
  } else if (loadcell.getSPS() > 100) {
    Serial.println("!!Sampling rate is higher than specification, check "
                   "MCU>HX711 wiring and pin designations");
  }
}

int SmartScale::updateWeight() {
  if (!loadcell.dataWaitingAsync()) {
    return weight;
  }
  loadcell.updateAsync();

  float measurement = loadcell.getData();
  measurement -= LOADCELL_MEASUREMENT_OFFSET;
  measurement = max(float(0), measurement * WEIGHT_SCALE_FACTOR);

  weight = int(measurement);
  return weight;
}

void SmartScale::setState(State *s) {
  if (state != s) {
    state = s;
    // state->reset(); // optional, if State has virtual reset()
  }
}

int SmartScale::getWeight() { return weight; }
