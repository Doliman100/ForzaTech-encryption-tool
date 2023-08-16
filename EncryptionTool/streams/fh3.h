#pragma once

#include <map>
#include "contexts/contexts.h"
#include "contexts/tables/fh5.h"
#include "contexts/tables/fh4.h"
#include "contexts/tables/fh3.h"
#include "contexts/tables/fm6apex.h"
#include "contexts/keys/fh5.h"
#include "contexts/keys/fh4.h"
#include "contexts/keys/fh3.h"
#include "streams/decryption_stream.h"

class ForzaHorizon3 : public DecryptionStream {
public:
  ForzaHorizon3(std::istream &input, uint32_t input_size);

  static std::vector<Context> contexts;
};
