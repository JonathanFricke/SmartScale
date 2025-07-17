#include "renderer.h"
#include "bitmaps.h"
#include "config.h"
#include "pins.h"

#include <Arduino.h>

Renderer::Renderer()
    : u8g2(U8G2_R0, DISPLAY_CLOCK_PIN, DISPLAY_DATA_PIN, DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RESET_PIN),
      animationCounter(0), animationSpeed(1.0), previousMillis(0), displayedWeight(0),
      displayDigitsBuffer(new byte[NUM_INTERNAL_PlACES]()) {
  u8g2.begin();
  u8g2.setContrast(DISPLAY_CONTRAST);

  for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
    displayIndicesShuffled[i] = i;
  }
  shuffleArray(displayIndicesShuffled);
}

Renderer &Renderer::getInstance() {
  static Renderer instance;
  return instance;
}

void Renderer::reset() {
  animationCounter = 0;
  animationSpeed = 1.0;
  shuffleArray(displayIndicesShuffled);

  // if (displayDigitsBuffer) {
  // 	delete[] displayDigitsBuffer;
  // 	displayDigitsBuffer = new byte[NUM_INTERNAL_PlACES]();
  // }
}

void Renderer::animWeight(int weight) {
  if (!isRenderDue())
    return;

  u8g2.setFont(u8g2_font_3x5im_te);
  u8g2.setDrawColor(1);
  u8g2.clearBuffer();
  intToDigits(weight, displayDigitsBuffer);
  for (int i = 0; i < NUM_INTERNAL_PlACES; i++) {
    Serial.print(displayDigitsBuffer[i]);
  }
  Serial.println("");
  renderDigitsBuffer(displayDigitsBuffer);
  u8g2.sendBuffer();
}

bool Renderer::animConfirm() {
  if (!isRenderDue(ANIM_TRANSITION_FRAME_TIME))
    return animationCounter >= (DISPLAY_WIDTH * DISPLAY_HEIGHT);

  u8g2.setDrawColor(1);
  renderConfirmTransition(animationCounter);
  u8g2.sendBuffer();

  if (animationCounter >= (DISPLAY_WIDTH * DISPLAY_HEIGHT)) {
    return true;
  } else {
    animationSpeed *= ANIM_TRANSITION_ACCELERATION;
    animationCounter += int(animationSpeed);
    animationCounter = min(animationCounter, DISPLAY_WIDTH * DISPLAY_HEIGHT);
    return false;
  }

  // if (animationCounter < (DISPLAY_WIDTH * DISPLAY_HEIGHT)) {
  //   animationSpeed *= ANIM_TRANSITION_ACCELERATION;
  //   animationCounter += int(animationSpeed);
  // } else {
  //   animationCounter = (DISPLAY_WIDTH * DISPLAY_HEIGHT);
  // }
}

bool Renderer::animBlink() {
  if (animationCounter > NUM_BLINK_FUNCTION_ITER) {
    return true;
  } else if (!isRenderDue(ANIM_BLINK_DURATION) && animationCounter > 0) // Allow for instant draw when first execute
    return false;

  u8g2.clearBuffer();
  if (animationCounter % 2 == 1) {
    u8g2.drawBox(DISPLAY_X_OFFSET, DISPLAY_Y_OFFSET, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  }
  u8g2.sendBuffer();
  animationCounter++;

  return false;
}

void Renderer::animLoading() {
  if (!isRenderDue(ANIM_LOADING_FRAME_TIME) &&
      animationCounter > NUM_BLINK_FUNCTION_ITER + 1) // Allow for instant draw when first execute
    return;
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(1);
  u8g2.drawBitmap(DISPLAY_X_OFFSET + BITMAP_LOADING_X_OFFSET, DISPLAY_Y_OFFSET, BITMAP_LOADING_WIDTH,
                  BITMAP_LOADING_HEIGHT, EPD_BITMAP_LOADING[animationCounter % NUM_LOADING_BITMAPS]);
  u8g2.sendBuffer();
  animationCounter++;
}

bool Renderer::renderCheckmark() {
  //   if (animationCounter > 0)
  //     return true;
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(1);
  u8g2.drawBitmap(DISPLAY_X_OFFSET + BITMAP_CHECKMARK_X_OFFSET, BITMAP_CHECKMARK_Y_OFFSET, BITMAP_CHECKMARK_WIDTH,
                  BITMAP_CHECKMARK_HEIGHT, EPD_BITMAP_CHECKMARK);
  u8g2.sendBuffer();
  return false;
}

bool Renderer::renderCross() {
  //   if (animationCounter > 0)
  //     return true;
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(1);
  u8g2.drawBitmap(DISPLAY_X_OFFSET + BITMAP_CHECKMARK_X_OFFSET, BITMAP_CHECKMARK_Y_OFFSET, BITMAP_CHECKMARK_WIDTH,
                  BITMAP_CHECKMARK_HEIGHT, EPD_BITMAP_CROSS);
  u8g2.sendBuffer();
  return false;
}

void Renderer::renderConfirmTransition(int currentCounter) {
  currentCounter = min(currentCounter, DISPLAY_WIDTH * DISPLAY_HEIGHT);
  for (int i = 0; i < currentCounter; i++) {
    Serial.print(displayIndicesShuffled[i]);
    Serial.print(" ");
    u8g2.drawPixel(DISPLAY_X_OFFSET + displayIndicesShuffled[i] % DISPLAY_WIDTH,
                   1 + displayIndicesShuffled[i] / DISPLAY_WIDTH);
  }
}

void Renderer::renderDigitsBuffer(const byte *digits) {
  const bool mostSignificantPlaceIsZero = digits[0] == 0 and digits[1] == 0 && HIDE_LEADING_ZERO;
  char digitStr[4];
  for (int i = 0; i < NUM_INTERNAL_PlACES; i++) {
    renderDecimalpoint(i, mostSignificantPlaceIsZero);

    // TODO: Make less significant digits random when weight is changing

    // if (i >= most_significant_zero_place) {
    //   itoa(random(0, 9), digitStr, 10);
    // } else {
    // Convert byte to string in base 10
    itoa(digits[i], digitStr, 10);
    // }

    const int x_pos = DISPLAY_X_OFFSET + (i - mostSignificantPlaceIsZero) * DISPLAY_CHAR_OFFSET;
    const int y_pos = DISPLAY_HEIGHT;

    u8g2.drawStr(x_pos, DISPLAY_HEIGHT, digitStr);
  }
}

void Renderer::renderDecimalpoint(int position, bool mostSignificantPlaceIsZero) {
  if (position + int(mostSignificantPlaceIsZero) == NUM_DISPLAY_PLACES - NUM_DECIMAL_PLACES) {
    u8g2.drawPixel(DISPLAY_X_OFFSET + (position + 1) * DISPLAY_CHAR_OFFSET - 1, DISPLAY_HEIGHT);
  }
}

void Renderer::shuffleArray(byte *arr, size_t size) {
  for (int i = 0; i < size; i++) {
    int randomIndex = random(size); // Pick a random index
    // Swap the current element with the element at the random index
    int temp = arr[i];
    arr[i] = arr[randomIndex];
    arr[randomIndex] = temp;
  }
}

bool Renderer::isRenderDue(unsigned long frameTime) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > frameTime) {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

void Renderer::intToDigits(int value, byte *out_digits) {
  for (int i = 0; i < NUM_INTERNAL_PlACES; ++i) {
    out_digits[i] = 0; // preset with zeros so we don’t need a second loop
  }

  for (int i = NUM_INTERNAL_PlACES - 1; i >= 0 && value >= 0; --i) {
    out_digits[i] = value % 10; // take the least‑significant digit
    value /= 10;
  }
}

int Renderer::mostSignificantZeroPlace(const byte *digits) {
  for (int i = 0; i < NUM_INTERNAL_PlACES; ++i) {
    if (digits[i] == 0) {
      return i; // position at digit
    }
  }
  return NUM_INTERNAL_PlACES - 1; // all digits are zero
}
