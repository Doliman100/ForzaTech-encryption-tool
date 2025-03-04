#pragma once

#include <stdint.h>
#include <ostream>

template<class It>
uint32_t Read32LE(It it) {
  uint32_t result = 0;
  for (size_t i = 0; i < 4; i++) {
    result |= static_cast<uint8_t>(*(it++)) << (8 * i);
  }
  return result;
}

template<class It>
uint32_t Read16LE(It it) {
  uint32_t result = 0;
  for (size_t i = 0; i < 2; i++) {
    result |= static_cast<uint8_t>(*(it++)) << (8 * i);
  }
  return result;
}

template<class It>
void Write32LE(uint32_t value, It it) {
  for (size_t i = 0; i < 4; i++) {
    *(it++) = value >> (8 * i);
  }
}

template<class It>
void Write16LE(uint32_t value, It it) {
  for (size_t i = 0; i < 2; i++) {
    *(it++) = value >> (8 * i);
  }
}

std::ostream &PrintBlock(std::ostream &os, std::array<uint8_t, 16> &data);
