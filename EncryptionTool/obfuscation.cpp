#include <ranges>
#include "obfuscation.h"

Obfuscation::Obfuscation(uint32_t seed, const std::array<uint8_t, 256> &crc32_mapping_table) :
  seed_(seed),
  crc32_mapping_table_(crc32_mapping_table) {

}

// 0000000140FF9050
uint32_t Obfuscation::CustomCRC32(uint32_t number) {
  uint32_t result = -1;
  for (size_t i = 0; i < 4; i++) {
    uint8_t low = result ^ crc32_mapping_table_[static_cast<uint8_t>(number >> (8 * i))]; // FM6Apex-FH4 don't cast the index to uint8_t
    result = (result >> 8) ^ crc32_lookup_table_[low];
  }
  return ~result;
}
