#pragma once
#include <Arduino.h>

// HX711-ADC
constexpr float LOADCELL_CALIBRATION_VALUE = 25712.00;
constexpr unsigned long LOADCELL_STABILIZING_TIME = 2000;
constexpr int LOADCELL_NUM_SAMPLES = 200; // Num of samples to average from
constexpr float LOADCELL_MEASUREMENT_OFFSET = 329.98;

// Display
constexpr bool HIDE_LEADING_ZERO = false; // |001.2|3 -> 0|01.23|
constexpr byte NUM_INTEGER_PLACES = 3;    // Max 999kg
constexpr byte NUM_DECIMAL_PLACES = 2;    // 999.99kg
constexpr int NUM_DISPLAY_PLACES = 4;     // [----]
constexpr byte DISPLAY_X_OFFSET = 16;     // x
constexpr byte DISPLAY_Y_OFFSET = 1;      // Px
constexpr byte DISPLAY_CHAR_OFFSET = 4;   // Px
constexpr int DISPLAY_CONTRAST = 255;     // default value
constexpr int DISPLAY_WIDTH = 16;         // Px
constexpr int DISPLAY_HEIGHT = 6;         // Px

constexpr int pow10(int n) { return (n == 0) ? 1 : 10 * pow10(n - 1); }
constexpr int WEIGHT_SCALE_FACTOR = pow10(NUM_DECIMAL_PLACES);
constexpr int NUM_INTERNAL_PlACES = NUM_INTEGER_PLACES + NUM_DECIMAL_PLACES;

// State
constexpr int MINIMAL_WEIGHT_KG = 1;              // Minimal recognized weight in kg
constexpr byte READING_TOLERANCE = 20;            // ±20g counts regarded as “no change”
constexpr int WEIGHT_STABILIZING_DURATION = 2500; // Duration in ms
constexpr int WEIGHT_CONFIRM_DURATION = 1000;     // Duration after transition animation in ms
constexpr int POST_RESULT_DURATION = 5000;        // Duration after transition animation in ms

constexpr int MINIMAL_WEIGHT = MINIMAL_WEIGHT_KG * 100; // Two decimal places in weight-int

// Animation
constexpr unsigned long ANIM_FRAME_TIME = 30;            // Frametime in ms
constexpr unsigned long ANIM_TRANSITION_FRAME_TIME = 50; // Slower animationspeed for transition
constexpr float ANIM_TRANSITION_ACCELERATION = 1.1;      // Animation speed in Px per iteration
constexpr unsigned long ANIM_BLINK_DURATION = 300;       // Duration of display flicker after confirmation in ms
constexpr unsigned long NUM_BLINKS = 1;                  // Blink is Off->wait->On->wait->...
constexpr unsigned long ANIM_LOADING_FRAME_TIME = 60;    // Slower animationspeed for transition
constexpr int BITMAP_LOADING_X_OFFSET = 4;               // Px offset for loading bitmaps
const int NUM_LOADING_BITMAPS = 8;                       // Loading-animation consists of 8 frames
constexpr int BITMAP_LOADING_WIDTH = 1;
constexpr int BITMAP_LOADING_HEIGHT = 5;
const int BITMAP_CHECKMARK_WIDTH = 1;
const int BITMAP_CHECKMARK_HEIGHT = 5;
const int BITMAP_CHECKMARK_X_OFFSET = 5;
const int BITMAP_CHECKMARK_Y_OFFSET = 1;

constexpr unsigned long NUM_BLINK_FUNCTION_ITER = NUM_BLINKS * 2;

// Network
constexpr bool SSL_INSECURE = false;  // Wether to use CA certificate or not (use on your own risk!)
constexpr int WIFI_RETRY_DELAY = 100; // Delay for retrying to connect to Wifi
constexpr int MAX_POST_ATTEMPTS = 3;
constexpr int POST_RETRY_DELAY_MS = 500;
constexpr int JSON_DOC_SIZE = 200; // Allocated memory for json-object to post
