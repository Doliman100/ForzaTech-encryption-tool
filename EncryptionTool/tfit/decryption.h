#pragma once

#include "tfit/tfit.h"

template<size_t Rounds>
class Decryption : public TFIT<Rounds> {
public:
  Decryption(std::array<uint8_t, 16> &previous, std::span<const std::array<std::array<uint8_t, 4>, 4>, Rounds> keys, std::array<std::array<std::array<std::array<uint8_t, 4>, 256>, 16>, Rounds> &tables);

  void Update(std::array<uint8_t, 16> &src, std::array<uint8_t, 16> &dst);

private:
  void RoundB(std::array<uint8_t, 16> &data, const std::array<std::array<uint8_t, 4>, 4> &key, std::array<std::array<std::array<uint8_t, 4>, 256>, 16> &table) override;
};
