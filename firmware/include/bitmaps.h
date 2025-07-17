#pragma once

#include "config.h"

// 'Loading_01', 8x5px
constexpr unsigned char EPD_BITMAP_LOADING_01[] = {0x08, 0x04, 0x7e, 0x04, 0x08};
// 'Loading_02', 8x5px
constexpr unsigned char EPD_BITMAP_LOADING_02[] = {0x18, 0x04, 0x7e, 0x04, 0x00};
// 'Loading_03', 8x5px
constexpr unsigned char EPD_BITMAP_LOADING_03[] = {0x18, 0x24, 0x7e, 0x00, 0x00};
// 'Loading_04', 8x5px
constexpr unsigned char EPD_BITMAP_LOADING_04[] = {0x18, 0x20, 0x7e, 0x20, 0x00};
// 'Loading_05', 8x5px
constexpr unsigned char EPD_BITMAP_LOADING_05[] = {0x10, 0x20, 0x7e, 0x20, 0x10};
// 'Loading_06', 8x5px
constexpr unsigned char EPD_BITMAP_LOADING_06[] = {0x00, 0x20, 0x7e, 0x20, 0x18};
// 'Loading_07', 8x5px
constexpr unsigned char EPD_BITMAP_LOADING_07[] = {0x00, 0x00, 0x7e, 0x24, 0x18};
// 'Loading_08', 8x5px
constexpr unsigned char EPD_BITMAP_LOADING_08[] = {0x00, 0x04, 0x7e, 0x24, 0x18};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 256)
const unsigned char *EPD_BITMAP_LOADING[NUM_LOADING_BITMAPS] = {
    EPD_BITMAP_LOADING_08, EPD_BITMAP_LOADING_07, EPD_BITMAP_LOADING_06, EPD_BITMAP_LOADING_05,
    EPD_BITMAP_LOADING_04, EPD_BITMAP_LOADING_03, EPD_BITMAP_LOADING_02, EPD_BITMAP_LOADING_01};

constexpr unsigned char EPD_BITMAP_CHECKMARK[] = {
    // 0xfc, 0xfa, 0x76, 0xae, 0xde
    0x02, 0x04, 0x88, 0x50, 0x20};

constexpr unsigned char EPD_BITMAP_CROSS[] = {0x28, 0x10, 0x28, 0x44, 0x44};
