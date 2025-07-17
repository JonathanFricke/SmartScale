#pragma once

#include "config.h"

#include <Arduino.h>
#include <U8g2lib.h>

class Renderer {
public:
  // Get the singleton instance
  static Renderer &getInstance();
  void reset();
  // TODO: Make animations class based and add them to states
  void animWeight(int weight);
  bool animConfirm();
  bool animBlink();
  void animLoading();
  bool renderCheckmark();
  bool renderCross();

private:
  Renderer();                                     // Private constructor for Singleton
  Renderer(const Renderer &) = delete;            // Prevent copy constructor (Renderer x = y will not work)
  Renderer &operator=(const Renderer &) = delete; // Prevent assignment (x = y will not work)

  bool isRenderDue(unsigned long frameTime = ANIM_FRAME_TIME);

  void renderDigitsBuffer(const byte *digits);
  void renderDecimalpoint(int i, bool mostSignificantPlaceIsZero);
  void renderConfirmTransition(int currentCounter);

  void intToDigits(int value, byte *out_digits);
  int mostSignificantZeroPlace(const byte *digits);
  void shuffleArray(byte *arr, size_t = DISPLAY_WIDTH * DISPLAY_HEIGHT);

  U8G2_MAX7219_32X8_F_4W_SW_SPI u8g2;
  int animationCounter;
  float animationSpeed;
  unsigned long previousMillis;
  int displayedWeight;
  byte *displayDigitsBuffer = nullptr;
  byte displayIndicesShuffled[16 * 6];
};
