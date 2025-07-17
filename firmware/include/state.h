#pragma once

class SmartScale;

class State {
public:
  // Virtual destructor to ensure proper cleanup of derived state classes when
  // deleted via a base class pointer
  virtual ~State() = default;
  // Pure virtual function
  virtual void step(SmartScale &scale) = 0;
  virtual void init(SmartScale &scale) {};

protected:
  virtual void changeState(SmartScale &scale, State &newState) = 0;
  virtual void reset() {
    counter = 0;
    millisTimestamp = 0;
  };
  int counter = 0;
  unsigned long millisTimestamp = 0;
};
