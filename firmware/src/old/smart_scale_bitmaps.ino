#include <Arduino.h>

// === CONSTANTS ===
const int CHECKMARK_WIDTH = 7;
const int CHECKMARK_HEIGHT = 5;
const int X_WIDTH = 7;
const int X_HEIGHT = 5;
const int LOADING_WIDTH = 8;
const int LOADING_HEIGHT = 5;
const int NUM_LOADING_BITMAPS = 8;

// 'checkmark', 7x5px
// const unsigned char epd_bitmap_checkmark [] PROGMEM = {
const unsigned char epd_bitmap_checkmark[] = {
    // 0xfc, 0xfa, 0x76, 0xae, 0xde
    0x02, 0x04, 0x88, 0x50, 0x20};

const unsigned char epd_bitmap_X[] = {0x28, 0x10, 0x28, 0x44, 0x44};

// 'Loading_04', 8x5px
const unsigned char epd_bitmap_Loading_04[] = {0x18, 0x20, 0x7e, 0x20, 0x00};
// 'Loading_05', 8x5px
const unsigned char epd_bitmap_Loading_05[] = {0x10, 0x20, 0x7e, 0x20, 0x10};
// 'Loading_06', 8x5px
const unsigned char epd_bitmap_Loading_06[] = {0x00, 0x20, 0x7e, 0x20, 0x18};
// 'Loading_07', 8x5px
const unsigned char epd_bitmap_Loading_07[] = {0x00, 0x00, 0x7e, 0x24, 0x18};
// 'Loading_02', 8x5px
const unsigned char epd_bitmap_Loading_02[] = {0x18, 0x04, 0x7e, 0x04, 0x00};
// 'Loading_08', 8x5px
const unsigned char epd_bitmap_Loading_08[] = {0x00, 0x04, 0x7e, 0x24, 0x18};
// 'Loading_03', 8x5px
const unsigned char epd_bitmap_Loading_03[] = {0x18, 0x24, 0x7e, 0x00, 0x00};
// 'Loading_01', 8x5px
const unsigned char epd_bitmap_Loading_01[] = {0x08, 0x04, 0x7e, 0x04, 0x08};

// Array of all bitmaps for convenience. (Total bytes used to store images in
// PROGMEM = 256)
const int epd_bitmap_Loading_LEN = NUM_LOADING_BITMAPS;
const unsigned char *epd_bitmap_Loading[NUM_LOADING_BITMAPS] = {
    epd_bitmap_Loading_08, epd_bitmap_Loading_07, epd_bitmap_Loading_06,
    epd_bitmap_Loading_05, epd_bitmap_Loading_04, epd_bitmap_Loading_03,
    epd_bitmap_Loading_02, epd_bitmap_Loading_01};
