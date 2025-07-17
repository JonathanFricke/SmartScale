#pragma once

#include "state.h"

class UploadState : public State {
public:
  static UploadState &getInstance();
  void step(SmartScale &scale) override;

private:
  void changeState(SmartScale &scale, State &newState) override;
  //   void init(SmartScale &scale) override;
  void reset() override;
  UploadState();                                        // Private constructor for Singleton
  UploadState(const UploadState &) = delete;            // Prevent copy constructor (Renderer x = y will not work)
  UploadState &operator=(const UploadState &) = delete; // Prevent assignment (x = y will not work)
};
