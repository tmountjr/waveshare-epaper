#include <pgmspace.h>

// FULL, >=66%
const uint8_t BatteryFull[16] = {
  0b00111111, 0b11111111,
  0b01000000, 0b00000001,
  0b10011101, 0b11011101,
  0b10011101, 0b11011101,
  0b10011101, 0b11011101,
  0b10011101, 0b11011101,
  0b01000000, 0b00000001,
  0b00111111, 0b11111111,
};

// MID, >= 33% and < 66%
const uint8_t BatteryMid[16] PROGMEM = {
  0b00111111, 0b11111111,
  0b01000000, 0b00000001,
  0b10000001, 0b11011101,
  0b10000001, 0b11011101,
  0b10000001, 0b11011101,
  0b10000001, 0b11011101,
  0b01000000, 0b00000001,
  0b00111111, 0b11111111,
};

// LOW, < 33%
const uint8_t BatteryLow[16] PROGMEM = {
  0b00111111, 0b11111111,
  0b01000000, 0b00000001,
  0b10000000, 0b00011101,
  0b10000000, 0b00011101,
  0b10000000, 0b00011101,
  0b10000000, 0b00011101,
  0b01000000, 0b00000001,
  0b00111111, 0b11111111,
};