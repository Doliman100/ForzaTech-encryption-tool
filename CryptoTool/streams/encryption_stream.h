#pragma once

#include <istream>
#include "contexts/contexts.h"
#include "tfit/mac.h"

class EncryptionStream {
public:
  EncryptionStream(std::ostream &os, uint32_t size, const std::array<uint8_t, 16> &iv, Context &context);

  void WriteData(std::istream &is);

protected:
  uint32_t size_;
  uint32_t padding_size_ = 0;
  uint32_t data_size_ = 0;

  Context &context_;
  std::array<uint8_t, 16> iv_; // TODO: use random iv
  std::ostream &os_;

  MAC mac_;
};
