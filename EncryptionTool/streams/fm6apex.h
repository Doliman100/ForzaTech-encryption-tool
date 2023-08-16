#pragma once

#include <map>
#include "contexts/contexts.h"
#include "contexts/tables/fm7.h"
#include "contexts/tables/fm6apex.h"
#include "contexts/keys/fm7.h"
#include "contexts/keys/fm6apex.h"
#include "streams/decryption_stream.h"

class ForzaMotorsport6Apex : public DecryptionStream {
public:
  ForzaMotorsport6Apex(std::istream &input, uint32_t input_size);

  static std::vector<Context> contexts;
};
