#pragma once

#include "state.h"

class MeasureState : public State {
public:
  static MeasureState &getInstance();
  void step(SmartScale &scale) override;

private:
  void changeState(SmartScale &scale, State &newState) override;
  //   void reset() override;
  MeasureState();                                         // Private constructor for Singleton
  MeasureState(const MeasureState &) = delete;            // Prevent copy constructor (Renderer x = y will not work)
  MeasureState &operator=(const MeasureState &) = delete; // Prevent assignment (x = y will not work)
};
