#pragma once

#include <array>
#include <span>

// Cipher
template<size_t Rounds>
class TFIT {
public:
  TFIT(std::array<uint8_t, 16> &previous, std::span<const std::array<std::array<uint8_t, 4>, 4>, Rounds> keys, std::array<std::array<std::array<std::array<uint8_t, 4>, 256>, 16>, Rounds> &tables);

protected:
  void RoundA(std::array<uint8_t, 16> &data, const std::array<std::array<uint8_t, 4>, 4> &key, std::array<std::array<std::array<uint8_t, 4>, 256>, 16> &table);
  virtual void RoundB(std::array<uint8_t, 16> &data, const std::array<std::array<uint8_t, 4>, 4> &key, std::array<std::array<std::array<uint8_t, 4>, 256>, 16> &table) = 0;
  void ProcessBlock(std::span<uint8_t, 16> src, std::array<uint8_t, 16> &dst);

  std::array<std::array<std::array<std::array<uint8_t, 4>, 256>, 16>, Rounds> &tables_;
  std::span<const std::array<std::array<uint8_t, 4>, 4>, Rounds> keys_;
  std::array<uint8_t, 16> previous_;
};
