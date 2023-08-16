#include <istream>
#include "iobinary.h"
#include "tfit/mac.h"
#include "streams/fm6apex.h"

ForzaMotorsport6Apex::ForzaMotorsport6Apex(std::istream &input, uint32_t input_size) :
  DecryptionStream(input) {
  if (input_size < 32) {
    return;
  }

  std::array<uint8_t, 16> header_mac{};
  std::array<uint8_t, 16> dst{};
  uint32_t block_size = 0x10;
  uint32_t data_block_size = 0;
  uint32_t data_size = 0;

  input.read(reinterpret_cast<char *>(iv_.data()), 16); // IV
  input.read(reinterpret_cast<char *>(header_mac.data()), 16); // header's MAC

  // <data_size> <IV>
  std::vector<uint8_t> header(4);
  header.reserve(18);
  header.insert(std::end(header), std::begin(iv_), std::end(iv_));

  for (auto &context : contexts) {
    // guess data_block_size. if data_size has remainder, it's wrong
    data_block_size = context.data_block_size;
    data_size = (input_size - 2 * block_size) / (data_block_size + block_size) * data_block_size;
    Write32LE(data_size, std::begin(header));

    MAC mac(context.mac_keys, context.mac_tables);
    mac.Calculate(header, dst);
    if (header_mac == dst) {
      context_ = std::shared_ptr<Context>(&context, [](void *) {});
      data_size_ = data_size;
      is_open_ = true;
      break;
    }
  }

  if (!is_open_) {
    input.seekg(-36, std::ios_base::cur);
  }
}

std::vector<Context> ForzaMotorsport6Apex::contexts = {
  {
    GameType::FM7,
    KeyType::ConfigFile,
    0x200,
    std::span(null_encryption_keys),
    std::span(fm7_configfile_decryption_keys).subspan<1, 17>(),
    std::span(fm7_configfile_mac_keys).subspan<1, 13>(),
    fm7_encryption_tables,
    fm7_decryption_tables,
    fm7_mac_tables
  },
  {
    GameType::FM7,
    KeyType::Profile,
    0x200,
    std::span(fm7_profile_encryption_keys).subspan<1, 17>(),
    std::span(fm7_profile_decryption_keys).subspan<1, 17>(),
    std::span(fm7_profile_mac_keys).subspan<1, 13>(),
    fm7_encryption_tables,
    fm7_decryption_tables,
    fm7_mac_tables
  },
  {
    GameType::FM7,
    KeyType::Reward,
    0x200,
    std::span(fm7_reward_encryption_keys).subspan<1, 17>(),
    std::span(fm7_reward_decryption_keys).subspan<1, 17>(),
    std::span(fm7_reward_mac_keys).subspan<1, 13>(),
    fm7_encryption_tables,
    fm7_decryption_tables,
    fm7_mac_tables
  },
  {
    GameType::FM7,
    KeyType::Photo,
    0x200,
    std::span(fm7_photo_encryption_keys).subspan<1, 17>(),
    std::span(fm7_photo_decryption_keys).subspan<1, 17>(),
    std::span(fm7_photo_mac_keys).subspan<1, 13>(),
    fm7_encryption_tables,
    fm7_decryption_tables,
    fm7_mac_tables
  },
  {
    GameType::FM7,
    KeyType::GameDB,
    0x20000,
    std::span(null_encryption_keys),
    std::span(fm7_gamedb_decryption_keys).subspan<1, 17>(),
    std::span(fm7_gamedb_mac_keys).subspan<1, 13>(),
    fm7_encryption_tables,
    fm7_decryption_tables,
    fm7_mac_tables,
    std::make_shared<Obfuscation>(fm7_db_obfuscation_seed, fm6apex_crc32_mapping)
  },
  {
    GameType::FM6Apex,
    KeyType::ConfigFile,
    0x200,
    std::span(null_encryption_keys),
    std::span(fm6apex_configfile_decryption_keys).subspan<1, 17>(),
    std::span(fm6apex_configfile_mac_keys).subspan<1, 13>(),
    fm6apex_encryption_tables,
    fm6apex_decryption_tables,
    fm6apex_mac_tables
  },
  {
    GameType::FM6Apex,
    KeyType::Profile,
    0x200,
    std::span(fm6apex_profile_encryption_keys).subspan<1, 17>(),
    std::span(fm6apex_profile_decryption_keys).subspan<1, 17>(),
    std::span(fm6apex_profile_mac_keys).subspan<1, 13>(),
    fm6apex_encryption_tables,
    fm6apex_decryption_tables,
    fm6apex_mac_tables
  },
  {
    GameType::FM6Apex,
    KeyType::Photo,
    0x200,
    std::span(fm6apex_photo_encryption_keys).subspan<1, 17>(),
    std::span(fm6apex_photo_decryption_keys).subspan<1, 17>(),
    std::span(fm6apex_photo_mac_keys).subspan<1, 13>(),
    fm6apex_encryption_tables,
    fm6apex_decryption_tables,
    fm6apex_mac_tables
  },
  {
    GameType::FM6Apex,
    KeyType::GameDB,
    0x20000,
    std::span(null_encryption_keys),
    std::span(fm6apex_gamedb_decryption_keys).subspan<1, 17>(),
    std::span(fm6apex_gamedb_mac_keys).subspan<1, 13>(),
    fm6apex_encryption_tables,
    fm6apex_decryption_tables,
    fm6apex_mac_tables,
    std::make_shared<Obfuscation>(fm6apex_db_obfuscation_seed, fm6apex_crc32_mapping)
  }
};
