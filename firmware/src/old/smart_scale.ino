#include <Arduino.h>

#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#include <U8g2lib.h>

#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include <esp_sleep.h>

#include "time.h"

// const char* ntpServer = "pool.ntp.org";
const char *ntpServer = "time.google.com";
const long gmtOffset_sec = 3600;     // Adjust for your timezone
const int daylightOffset_sec = 3600; // Adjust for DST if needed

#include <secret.h>

// === CONSTANTS ===
// Pin assignments
const int HX711_DOUT_PIN = 0; // HX711 dout pin
const int HX711_SCK_PIN = 1;  // HX711 sck pin
const int DISPLAY_CLOCK_PIN = 19;
const int DISPLAY_DATA_PIN = 18;
const int DISPLAY_CS_PIN = 20;
const int DISPLAY_DC_PIN = U8X8_PIN_NONE;
const int DISPLAY_RESET_PIN = U8X8_PIN_NONE;
const int BUTTON_PIN = 2;

// Calibration and EEPROM
const int CALIBRATION_EEPROM_ADDRESS = 0;
const float LOADCELL_CALIBRATION_VALUE = 25712.00;
const unsigned long LOADCELL_STABILIZING_TIME = 2000;
const int LOADCELL_NUM_SAMPLES = 200;
const float LOADCELL_MEASUREMENT_OFFSET = 329.98;

// Display
const byte DISPLAY_X_OFFSET = 16;
const byte DISPLAY_Y_OFFSET = 6;
const byte DISPLAY_CHAR_OFFSET = 4;
const int DISPLAY_DIGIT_ANIM_DURATION = 10;
const int DISPLAY_CONTRAST = 10 * 16; // 160

// Weight
const int MINIMUM_WEIGHT = 100;
const int NUM_DIGITS = 5;

// Timing
const int LOADCELL_INTERVAL = 1000;
const int STABLE_INPUT_SUBMIT_DELAY = 100;
const float ANIM_FRAME_TIME = 30.0;
const float TO_SLEEP_TIMER = 10 * 1000;

// Sleep
#define WAKEUP_BUTTON_PIN_BITMASK (1ULL << GPIO_NUM_2) // GPIO 2 bitmask for ext1

// HX711_ADC
// const int HX711_dout = 23; // HX711 dout pin
// const int HX711_sck = 16; // HX711 sck pin
const int HX711_dout = HX711_DOUT_PIN; // HX711 dout pin
const int HX711_sck = HX711_SCK_PIN;   // HX711 sck pin
HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_calVal_eepromAdress = CALIBRATION_EEPROM_ADDRESS;

const int loadCell_intervall = LOADCELL_INTERVAL; // increase value to slow down serial print activity
boolean _tare = false;                            // set this to false if you don't want tare to be
                                                  // performed in the next step
const int num_samples = LOADCELL_NUM_SAMPLES;

int input_raw = 0;
int input = 0;

// U8G2 matrix display
U8G2_MAX7219_32X8_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/DISPLAY_CLOCK_PIN,
                                   /* data=*/DISPLAY_DATA_PIN,
                                   /* cs=*/DISPLAY_CS_PIN,
                                   /* dc=*/DISPLAY_DC_PIN,
                                   /* reset=*/DISPLAY_RESET_PIN);

int current = 0;
int stable_input = 0;

unsigned long previousMillis = 0;

byte display_fill[16 * 6];

bool finished_routine = false;

byte diff_byte_arr[NUM_DIGITS] = {0, 0, 0, 0, 0};
int most_significant_zero_place = 0;

// Either 123.4 or 23.45 is shown but need to store 123.45
byte digits[NUM_DIGITS] = {0, 0, 0, 0, 0};
// Temporary character to store digit and null (as end-token)
char digit_str[2];
const int num_digits = sizeof(digits);

const int minimum_weight = MINIMUM_WEIGHT;

enum state { measure, confirm, upload, download, stop };
state status;

const byte x_offset_abs = DISPLAY_X_OFFSET;
const byte y_offset_abs = DISPLAY_Y_OFFSET;
const byte char_offset = DISPLAY_CHAR_OFFSET;
const int digit_anim_duration = DISPLAY_DIGIT_ANIM_DURATION;
const int stable_input_submit_delay = STABLE_INPUT_SUBMIT_DELAY;
const float anim_frame_time = ANIM_FRAME_TIME;

bool pressed = false;

const float to_sleep_timer = TO_SLEEP_TIMER;
long last_input_change_time = millis();
#define WAKEUP_BUTTON_PIN_BITMASK (1ULL << GPIO_NUM_2) // GPIO 2 bitmask for ext1
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

void setup() {
  Serial.begin(9600);

  setupLoadCell(LOADCELL_CALIBRATION_VALUE, LOADCELL_STABILIZING_TIME);
  setupDisplay();
  setupWifi();

  last_input_change_time = millis();
  // Print the wakeup reason for ESP32
  print_wakeup_reason();
}

void setupDisplay() {
  state status = measure;
  u8g2.begin();                       // begin function is required for u8g2 library
  u8g2.setContrast(DISPLAY_CONTRAST); // set display contrast 0-255

  for (int i = 0; i < sizeof(display_fill); i++) {
    display_fill[i] = i;
  }
  shuffleDisplayFill();

  pinMode(BUTTON_PIN, INPUT);
}

void setupLoadCell(float calibrationValue, unsigned long stabilizingtime) {
  LoadCell.powerUp();
  LoadCell.begin();
  LoadCell.start(stabilizingtime, _tare);
  LoadCell.setSamplesInUse(LOADCELL_NUM_SAMPLES);

  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  } else {
    LoadCell.setCalFactor(calibrationValue); // set calibration factor (float)
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update())
    ;
  Serial.print("Calibration value: ");
  Serial.println(LoadCell.getCalFactor());
  Serial.print("HX711 measured conversion time ms: ");
  Serial.println(LoadCell.getConversionTime());
  Serial.print("HX711 measured sampling rate HZ: ");
  Serial.println(LoadCell.getSPS());
  Serial.print("HX711 measured settlingtime ms: ");
  Serial.println(LoadCell.getSettlingTime());
  Serial.println("Note that the settling time may increase significantly if "
                 "you use delay() in your sketch!");
  if (LoadCell.getSPS() < 7) {
    Serial.println("!!Sampling rate is lower than specification, check "
                   "MCU>HX711 wiring and pin designations");
  } else if (LoadCell.getSPS() > 100) {
    Serial.println("!!Sampling rate is higher than specification, check "
                   "MCU>HX711 wiring and pin designations");
  }
}

int t = 0;
void loop() {
  unsigned long currentMillis = millis();
  getLoadCellValue_blocking();
  // Serial.print("input: ");
  // Serial.println(input);

  draw(currentMillis);
  tryEnterSleepMode(currentMillis);
  t++;
}

void getLoadCellValue_blocking() {
  if (!LoadCell.update()) {
    return;
  }

  float measurement_raw = LoadCell.getData();
  if (_tare == false) {
    measurement_raw -= LOADCELL_MEASUREMENT_OFFSET;
  }

  input_raw = max(float(0), measurement_raw * 100);
  if (input_raw == 0.00) {
    setInput(input_raw);
  } else {
    setInput(input_raw);
  }
}

void getLoadCellValue_Nonblocking() {
  if (!LoadCell.dataWaitingAsync()) { // Check if new data is available
    return;
  }
  LoadCell.updateAsync(); // Update with new data (non-blocking)

  float measurement_raw = LoadCell.getData();
  if (_tare == false) {
    measurement_raw -= LOADCELL_MEASUREMENT_OFFSET;
  }

  input_raw = max(float(0), measurement_raw * 100);
  if (input_raw == 0.00) {
    setInput(input_raw);
  } else {
    setInput(input * 0.9 + input_raw * 0.1);
  }
}

void tryEnterSleepMode(long currentMillis) {
  if (status != measure || (status == measure && abs(input - current) > 10)) {
    last_input_change_time = millis();
  }

  if (currentMillis - last_input_change_time < to_sleep_timer) {
    return;
  }

  enterSleepMode();
}

void enterSleepMode() {
  esp_sleep_enable_ext1_wakeup(WAKEUP_BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  Serial.println("Going to sleep now");
  WiFi.disconnect();
  u8g2.setPowerSave(1);
  LoadCell.powerDown();
  esp_deep_sleep_start();
}
