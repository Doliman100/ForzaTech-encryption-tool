#pragma once

#include <array>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <vector>
#include <boost/any.hpp>
#include "obfuscation.h"

// How to extract keys using x64dbg.
// 1. find first row of bytes from table that you interested it and place breakpoint on the first of them. For decryption is fh4_mac_tables[0]=fh5_decryption_tables_memory[7].
// 2. 

// mapping
// TODO:
// consexpr array of indices
//   constexpr function to make consexpr array from _memory[index]

enum class KeyType {
  Profile,
  Reward,
  GameDB,
  ConfigFile,
  File,
  SFS,
  Photo,
  Dynamic,
  Telemetry
};

enum class GameType {
  FH5,
  FH4,
  FM7,
  FH3,
  FM6Apex
};

//enum class EncryptionType {
//  Yes,
//  No,
//  None
//};

struct Context {
  GameType game_type;
  KeyType key_type;
  uint32_t data_block_size;
  std::span<const std::array<std::array<uint8_t, 4>, 4>, 17> encryption_keys;
  std::span<const std::array<std::array<uint8_t, 4>, 4>, 17> decryption_keys;
  std::span<const std::array<std::array<uint8_t, 4>, 4>, 13> mac_keys;
  std::array<std::array<std::array<std::array<uint8_t, 4>, 256>, 16>, 17> &encryption_tables;
  std::array<std::array<std::array<std::array<uint8_t, 4>, 256>, 16>, 17> &decryption_tables;
  std::array<std::array<std::array<std::array<uint8_t, 4>, 256>, 16>, 13> &mac_tables;
  std::shared_ptr<Obfuscation> obfuscation; // optional
};

struct Name {
  std::string full;
  std::string abbreviation;
};

extern std::map<KeyType, Name> key_types;
extern std::map<GameType, Name> game_types;

constexpr std::array<std::array<std::array<uint8_t, 4>, 4>, 17> null_encryption_keys = {};

void validate(boost::any &v, const std::vector<std::string> &values, GameType *target_type, int);
void validate(boost::any &v, const std::vector<std::string> &values, KeyType *target_type, int);
