#pragma once

#include "internetClient.h"
#include "pins.h"
#include "renderer.h"
#include "state.h"

#include <Arduino.h>
#include <HX711_ADC.h>

class SmartScale {
public:
  // Implement Constructor and Destructor by ourself
  SmartScale();
  ~SmartScale();

  void init();
  void step();
  int getWeight();
  int updateWeight();
  void setState(State *s);
  Renderer *renderer = nullptr;
  InternetClient *internetClient = nullptr;

private:
  void setupLoadcell();

  State *state = nullptr;
  HX711_ADC loadcell;

  // TODO: Add deepsleep

  int weight;
};
