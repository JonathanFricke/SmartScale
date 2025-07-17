#pragma once

#include "state.h"

class ConfirmState : public State {
public:
  static ConfirmState &getInstance();
  void step(SmartScale &scale) override;

private:
  void changeState(SmartScale &scale, State &newState) override;
  void init(SmartScale &scale) override;
  ConfirmState();                                         // Private constructor for Singleton
  ConfirmState(const ConfirmState &) = delete;            // Prevent copy constructor (Renderer x = y will not work)
  ConfirmState &operator=(const ConfirmState &) = delete; // Prevent assignment (x = y will not work)
  int init_weight;
};
