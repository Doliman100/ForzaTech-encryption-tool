#include <iostream>
#include <fstream>
#include <ranges>
#include <vector>
#include "tfit/decryption.h"
#include "contexts/contexts.h"
#include "iobinary.h"
#include "streams/decryption_stream.h"

DecryptionStream::DecryptionStream(std::istream &input) :
  input_(input),
  iv_() {

}

void DecryptionStream::ReadData(std::ostream &output) {
  std::array<uint8_t, 16> src{};
  std::array<uint8_t, 16> dst{};

  std::vector<uint8_t> dst_data_block(context_->data_block_size);

  Decryption<17> cipher(iv_, context_->decryption_keys, context_->decryption_tables);
  for (size_t offset = 0; offset < data_size_; offset += context_->data_block_size) {
    auto dst_data_block_it = std::begin(dst_data_block);
    for (size_t i = 0; i < context_->data_block_size / 16; i++) {
      input_.read(reinterpret_cast<char *>(src.data()), 16);
      cipher.Update(src, dst);
      dst_data_block_it = std::copy(std::begin(dst), std::end(dst), dst_data_block_it);
    }

    // MAC is also part of an encrypted flow, so process it but don't print
    input_.read(reinterpret_cast<char *>(src.data()), 16);
    cipher.Update(src, dst);

    if (context_->obfuscation) {
      context_->obfuscation->Obfuscate(offset, dst_data_block);
    }

    if (offset + context_->data_block_size == data_size_ && padding_size_) {
      output.write(reinterpret_cast<char *>(dst_data_block.data()), context_->data_block_size - padding_size_);
    } else {
      output.write(reinterpret_cast<char *>(dst_data_block.data()), context_->data_block_size);
    }
  }
}
