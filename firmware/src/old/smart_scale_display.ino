#include <Arduino.h>
int t_anim = 0;
int stable_input_t = 0;
int input_changed_t = 0;
void draw(long currentMillis) {
  if (currentMillis - previousMillis < anim_frame_time) {
    return;
  }
  previousMillis = currentMillis;
  u8g2.clearBuffer();

  anim_weight();
  anim_confirm();
  anim_store();
  anim_weight_diff();

  if (input < minimum_weight) {
    finished_routine = false;
  }

  pressed = false;
  u8g2.sendBuffer();
  t_anim++;
}

// === CONSTANTS ===
const int MIN_WEIGHT_DIFF = 10;
const int DECIMAL_PLACES = 3;
const int DISPLAY_WIDTH = 16;
const int DISPLAY_HEIGHT = 6;
const int BITMAP_LOADING_HEIGHT = 1;
const int BITMAP_LOADING_HEIGHT = 5;
const int BITMAP_LOADING_X_OFFSET = 5;
const int BITMAP_CHECKMARK_Y_OFFSET = 1;
const int MAX_SEND_ATTEMPTS = 2;
const int MAX_RECEIVE_ATTEMPTS = 2;
const int CONFIRM_WEIGHT_MAX_DEVIATION = 9;
const float PIXEL_PER_ITER = 0.5;
const float PIXEL_PER_ITER_SCALER = 1.05;
const int CONFIRM_DURATION = 15;
const int FLICKER_DURATION = 10;
const int SHOW_UPLOAD_STATUS_DURATION = 50;
const int PRESSED_DELAY = 100;
const int T_SHOW_DIFFERENCE_DURATION = 100;
const int LOADING_ANIM_MOD = 3;

void anim_weight() {
  if (status != measure && status != confirm) {
    return;
  }
  u8g2.setFont(u8g2_font_3x5im_te);
  u8g2.setDrawColor(1);

  if (abs(input - current) > MIN_WEIGHT_DIFF) {
    float diff = input - current;
    most_significant_zero_place = intToByteArray(abs(diff), diff_byte_arr);

    stable_input_t = 0;
  } else {
    most_significant_zero_place = num_digits;
    intToByteArray(input, digits);

    if (stable_input_t >= stable_input_submit_delay && input >= minimum_weight && !finished_routine &&
        status != confirm) {
      // Serial.print("STABLE:");
      // Serial.println(input);
      stable_input = input;
      status = confirm;
    }
    stable_input_t++;
  }

  for (int i = num_digits - 1; i >= 0; i--) {
    draw_weight(i);
  }

  if ((t_anim - input_changed_t) % digit_anim_duration == 0) {
    current = input;
  }
}

void draw_weight(byte i) {
  bool most_significant_is_zero = digits[0] == 0 ? true : false;

  // Decimal point at position 3
  if (i == DECIMAL_PLACES) {
    u8g2.drawPixel(x_offset_abs + (i - most_significant_is_zero) * char_offset - 1, DISPLAY_Y_OFFSET);
  }

  if (i >= most_significant_zero_place + 1) {
    itoa(random(0, 9), digit_str, 10);
  } else {
    // Convert byte to string in base 10
    itoa(digits[i], digit_str, 10);
  }

  int x_pos = x_offset_abs + (i - most_significant_is_zero) * char_offset;
  int y_pos = y_offset_abs;

  u8g2.drawStr(x_pos, y_pos, digit_str);
}

float pixel_per_iter_scaled = PIXEL_PER_ITER;
float anim_fill_t = 0;
int t_confirm = 0;
void anim_confirm() {
  if (status != confirm) {
    return;
  }

  if (abs(input - stable_input) > CONFIRM_WEIGHT_MAX_DEVIATION) {
    anim_fill_t = 0;
    pixel_per_iter_scaled = PIXEL_PER_ITER;
    status = measure;
  }

  u8g2.setDrawColor(1);
  for (int i = 0; i < int(min((int)round(anim_fill_t), DISPLAY_WIDTH * DISPLAY_HEIGHT)); i++) {
    u8g2.drawPixel(x_offset_abs + display_fill[i] % DISPLAY_WIDTH, 1 + display_fill[i] / DISPLAY_WIDTH);
  }

  if (anim_fill_t >= DISPLAY_WIDTH * DISPLAY_HEIGHT) {
    if (t_confirm >= CONFIRM_DURATION) {
      anim_fill_t = 0;
      t_confirm = 0;
      pixel_per_iter_scaled = PIXEL_PER_ITER;
      shuffleDisplayFill();
      status = upload;
    }
    t_confirm++;
  } else {
    pixel_per_iter_scaled *= PIXEL_PER_ITER_SCALER;
    anim_fill_t += pixel_per_iter_scaled;
  }
}

int t_flicker = 0;
bool send_data = false;
int t_show_upload_status = 0;

int t_pressed = 0;
int send_attempt = 0;

void anim_store() {
  if (status != upload) {
    return;
  }

  // Quick flicker (confirmation)
  if (t_flicker < FLICKER_DURATION) {
    t_flicker++;
  } else if (t_flicker < FLICKER_DURATION * 2) {
    u8g2.drawBox(x_offset_abs, 1, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    for (int i = DISPLAY_WIDTH * DISPLAY_HEIGHT - anim_fill_t; i >= 0; i--) {
      u8g2.drawPixel(x_offset_abs + display_fill[i] % DISPLAY_WIDTH, 1 + display_fill[i] / DISPLAY_WIDTH);
    }
    t_flicker++;
  } else {
    if (send_attempt < MAX_SEND_ATTEMPTS && !send_data) {
      int post_result = postWeight(stable_input);
      if (post_result == 200) {
        send_data = true;
      }
      send_attempt++;
    }

    if (send_data) {
      if (t_show_upload_status >= SHOW_UPLOAD_STATUS_DURATION) {
        status = download;
        t_flicker = 0;
        t_show_upload_status = 0;
        send_data = false;
        send_attempt = 0;
        return;
      }
      u8g2.setDrawColor(1);
      u8g2.setBitmapMode(1);
      u8g2.drawBitmap(x_offset_abs + BITMAP_LOADING_X_OFFSET, BITMAP_CHECKMARK_Y_OFFSET, BITMAP_LOADING_HEIGHT,
                      BITMAP_LOADING_HEIGHT, epd_bitmap_checkmark);

      t_show_upload_status++;
    } else {
      if (t_show_upload_status >= SHOW_UPLOAD_STATUS_DURATION) {
        status = measure;
        t_flicker = 0;
        t_show_upload_status = 0;
        send_data = false;
        send_attempt = 0;
        return;
      }
      u8g2.setDrawColor(1);
      u8g2.setBitmapMode(1);
      u8g2.drawBitmap(x_offset_abs + BITMAP_LOADING_X_OFFSET, BITMAP_CHECKMARK_Y_OFFSET, BITMAP_LOADING_HEIGHT,
                      BITMAP_LOADING_HEIGHT, epd_bitmap_X);

      t_show_upload_status++;
      // draw_loading();
    }
  }
}

int t_show_difference = 0;
bool receive_data = false;

int receive_attempt = 0;
int last_stored_weight;

void anim_weight_diff() {
  if (status != download) {
    return;
  }

  if (t_show_difference >= T_SHOW_DIFFERENCE_DURATION) {
    finished_routine = true;
    status = measure;
    t_show_difference = 0;
    receive_data = false;
    t_show_upload_status = 0;
    t_pressed = 0;
    receive_attempt = 0;
    return;
  }

  if (receive_attempt < MAX_RECEIVE_ATTEMPTS && !receive_data) {
    last_stored_weight = getWeight();
    if (last_stored_weight > 0) {
      receive_data = true;
    }
    receive_attempt++;
  }

  if (receive_data) {
    u8g2.setDrawColor(1);
    // u8g2.drawStr(x_offset_abs, y_offset_abs, "-1.69");
    draw_weight_diff((float(stable_input) / 100.0) - (float(last_stored_weight) / 100.0));
    t_show_difference++;
  } else {
    u8g2.setDrawColor(1);
    u8g2.setBitmapMode(1);
    u8g2.drawBitmap(x_offset_abs + BITMAP_LOADING_X_OFFSET, BITMAP_CHECKMARK_Y_OFFSET, BITMAP_LOADING_HEIGHT,
                    BITMAP_LOADING_HEIGHT, epd_bitmap_X);
    t_show_difference++;
  }

  t_show_upload_status++;
}

void draw_weight_diff(float diff_) {
  int diff = abs(diff_ * 100);
  byte weight_diff[NUM_DIGITS];
  intToByteArray(diff, weight_diff);
  bool is_leading_zero = true;

  int offset_ = 0;
  for (int i = -1; i < NUM_DIGITS; i++) {
    if (i == -1) {
      if (diff_ < 0) {
        u8g2.drawStr(x_offset_abs, y_offset_abs, "-");
      } else {
        u8g2.drawStr(x_offset_abs, y_offset_abs, "+");
      }
      offset_ = char_offset;
      continue;
    }

    if (weight_diff[i] == 0 && is_leading_zero && i < 2) {
      continue;
    }

    if (i == DECIMAL_PLACES) {
      u8g2.drawPixel(x_offset_abs + offset_ - 1, DISPLAY_Y_OFFSET);
    }

    // Convert byte to string in base 10
    itoa(weight_diff[i], digit_str, 10);
    u8g2.drawStr(x_offset_abs + offset_, y_offset_abs, digit_str);
    is_leading_zero = false;
    offset_ += char_offset;
  }
}

int t_loading = 0;
void draw_loading() {
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(1);
  u8g2.drawBitmap(x_offset_abs + 4, 1, BITMAP_LOADING_HEIGHT, BITMAP_LOADING_HEIGHT, epd_bitmap_Loading[t_loading]);
  if (t_anim % LOADING_ANIM_MOD == 0) {
    t_loading = (t_loading + 1) % epd_bitmap_Loading_LEN;
  }
}

void setInput(float input_) {
  if (abs(input_ - input) > MIN_WEIGHT_DIFF) {
    input_changed_t = t_anim - 1;
  }
  input = input_;
}

void shuffleDisplayFill() {
  for (int i = 0; i < sizeof(display_fill); i++) {
    int randomIndex = random(sizeof(display_fill)); // Pick a random index
    // Swap the current element with the element at the random index
    int temp = display_fill[i];
    display_fill[i] = display_fill[randomIndex];
    display_fill[randomIndex] = temp;
  }
}

int byteArrToInt(byte arr[]) {
  int x = 0;
  for (int i = 0; i < NUM_DIGITS; i++) {
    x = x * 10 + arr[i]; // Shift left by one decimal place and add the digit
  }
  return x;
}

int intToByteArray(int x, byte *result) {
  int most_significant_zero_place = -1;
  for (int i = NUM_DIGITS - 1; i >= 0; i--) {
    if (x == 0) {
      most_significant_zero_place = max(most_significant_zero_place, i);
    }

    result[i] = x % 10; // Get the last digit
    x /= 10;            // Remove the last digit
  }
  return most_significant_zero_place;
}
