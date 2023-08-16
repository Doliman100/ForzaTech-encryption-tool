#pragma once

#include <string>
#include <array>
#include <memory>
#include "contexts/contexts.h"

class DecryptionStream {
public:
  DecryptionStream(std::istream &input);

  void ReadData(std::ostream &output);

  std::string &game_type() { return game_types.at(context_->game_type).full; }
  std::string &key_type() { return key_types.at(context_->key_type).full; }
  uint32_t data_block_size() { return context_->data_block_size; }
  uint32_t padding_size() { return padding_size_; }
  uint32_t data_size() { return data_size_; }
  uint32_t decrypted_file_size() { return data_size_ - padding_size_; }
  std::array<uint8_t, 16> &iv() { return iv_; }
  // IsEncrypted
  // is match this format (Forza Horizon or Forza Motorsport). is supported?

  operator bool() { return is_open_; }

protected:
  bool is_open_ = false;
  uint32_t padding_size_ = 0;
  uint32_t data_size_ = 0;

  std::shared_ptr<Context> context_;
  std::array<uint8_t, 16> iv_;
  std::istream &input_;
};
