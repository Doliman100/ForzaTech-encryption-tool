#include <iostream>
#include <vector>
#include "iobinary.h"
#include "tfit/encryption.h"
#include "tfit/mac.h"
#include "streams/encryption_stream.h"

EncryptionStream::EncryptionStream(std::ostream &os, uint32_t size, const std::array<uint8_t, 16> &iv, Context &context)
  : os_(os)
  , iv_(iv)
  , context_(context)
  , size_(size)
  , mac_(context.mac_keys, context.mac_tables) {
  padding_size_ = context.data_block_size - size_ % context.data_block_size;
  data_size_ = size_ + padding_size_;

  std::vector<uint8_t> header;
  Write32LE(data_size_, std::back_inserter(header));
  header.insert(std::end(header), std::begin(iv_), std::end(iv_));
  Write32LE(padding_size_, std::back_inserter(header));
  os.write(reinterpret_cast<char *>(header.data()) + 4, 20);

  std::array<uint8_t, 16> header_mac{};
  mac_.Calculate(header, header_mac);
  os.write(reinterpret_cast<char *>(header_mac.data()), 16);
}

void EncryptionStream::WriteData(std::istream &is) {
  std::array<uint8_t, 16> src{};
  std::array<uint8_t, 16> dst{};
  std::array<uint8_t, 16> mac{};

  std::vector<uint8_t> src_data_block(context_.data_block_size);

  Encryption<17> cipher(iv_, context_.encryption_keys, context_.encryption_tables);
  for (size_t offset = 0; offset < data_size_; offset += context_.data_block_size) { // | enumerate | chunk
    if (offset + context_.data_block_size == data_size_ && padding_size_) {
      is.read(reinterpret_cast<char *>(src_data_block.data()), src_data_block.size() - padding_size_);
      std::fill(std::end(src_data_block) - padding_size_, std::end(src_data_block), 0);
    } else {
      is.read(reinterpret_cast<char *>(src_data_block.data()), src_data_block.size());
    }

    //if (context_.obfuscation) {
    //  context_.obfuscation->Obfuscate(offset, src_data_block);
    //}

    auto src_data_block_it = std::begin(src_data_block);
    for (size_t i = 0; i < context_.data_block_size / 16; i++) {
      std::copy_n(src_data_block_it, 16, std::begin(src));
      cipher.Update(src, dst);
      std::advance(src_data_block_it, 16);
      os_.write(reinterpret_cast<char *>(dst.data()), 16);
    }

    mac_.Calculate(src_data_block, mac);
    cipher.Update(mac, dst);
    os_.write(reinterpret_cast<char *>(dst.data()), 16);
  }
}
