#pragma once

#include <vector>
#include "tfit/encryption.h"

// don't inherit Encryption, create it inside Calculate function
class MAC : public Encryption<13> {
public:
  MAC(std::span<const std::array<std::array<uint8_t, 4>, 4>, 13> keys, std::array<std::array<std::array<std::array<uint8_t, 4>, 256>, 16>, 13> &tables);

  void Calculate(std::vector<uint8_t> &data, std::array<uint8_t, 16> &dst);

private:
  void CircularShift(std::array<uint8_t, 16> &src, std::array<uint8_t, 16> &dst);

  std::array<uint8_t, 16> iv_placeholder_;
};
