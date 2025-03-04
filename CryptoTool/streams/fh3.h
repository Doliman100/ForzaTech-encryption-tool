#pragma once

#include <map>
#include "contexts/contexts.h"
#include "streams/decryption_stream.h"
#include "streams/encryption_stream.h"

namespace ForzaHorizon3 {
  class DecryptionStream : public ::DecryptionStream {
  public:
    DecryptionStream(std::istream & input, uint32_t input_size);
  };

  class EncryptionStream : public ::EncryptionStream {
  public:
    EncryptionStream(std::ostream &os, uint32_t size, const std::array<uint8_t, 16> &iv, Context &context);
  };

  extern std::vector<Context> contexts;
}
