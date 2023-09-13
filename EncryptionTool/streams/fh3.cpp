#include <istream>
#include "iobinary.h"
#include "contexts/tables/fh5.h"
#include "contexts/tables/fh4.h"
#include "contexts/tables/fh3.h"
#include "contexts/tables/fm6apex.h"
#include "contexts/keys/fh5_v1.614.70.0.h"
#include "contexts/keys/fh5.h"
#include "contexts/keys/fh4.h"
#include "contexts/keys/fh3.h"
#include "tfit/mac.h"
#include "streams/fh3.h"

ForzaHorizon3::DecryptionStream::DecryptionStream(std::istream &input, uint32_t input_size)
  : ::DecryptionStream(input) {
  if (input_size < 36) {
    return;
  }

  std::array<uint8_t, 16> header_mac{};
  std::array<uint8_t, 16> dst{};
  uint32_t block_size = 0x10;
  uint32_t data_block_size = 0;
  uint32_t data_size = 0;

  input.read(reinterpret_cast<char *>(iv_.data()), 16); // IV
  padding_size_ = Read32LE(std::istreambuf_iterator<char>(input));
  input.read(reinterpret_cast<char *>(header_mac.data()), 16);

  // <data_size> <IV> <padding_size>
  std::vector<uint8_t> header;
  std::fill_n(std::back_inserter(header), 4, 0);
  header.insert(std::end(header), std::begin(iv_), std::end(iv_));
  Write32LE(padding_size_, std::back_inserter(header));

  for (auto &context : contexts) {
    // guess data_block_size. if data_size has remainder, it's wrong
    data_block_size = context.data_block_size;
    data_size = (input_size - (2 * block_size + 4)) / (data_block_size + block_size) * data_block_size;
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

ForzaHorizon3::EncryptionStream::EncryptionStream(std::ostream &os, uint32_t size, const std::array<uint8_t, 16> &iv, Context &context)
  : ::EncryptionStream(os, size, iv, context) {
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

std::vector<Context> ForzaHorizon3::contexts = {
  {
    GameType::FH5_v1_614_70_0,
    KeyType::File,
    0x200,
    std::span(null_encryption_keys),
    std::span(fh5_v1_614_70_0_file_decryption_keys).subspan<1, 17>(),
    std::span(fh5_v1_614_70_0_file_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH5_v1_614_70_0,
    KeyType::GameDB,
    0x20000,
    std::span(null_encryption_keys),
    std::span(fh5_v1_614_70_0_gamedb_decryption_keys).subspan<1, 17>(),
    std::span(fh5_v1_614_70_0_gamedb_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables,
    std::make_shared<Obfuscation>(fh5_v1_614_70_0_gamedb_obfuscation_seed, fh5_crc32_mapping)
  },
  {
    GameType::FH5_v1_614_70_0,
    KeyType::SFS,
    0x20000,
    std::span(null_encryption_keys),
    std::span(fh5_v1_614_70_0_sfs_decryption_keys).subspan<1, 17>(),
    std::span(fh5_v1_614_70_0_sfs_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH5,
    KeyType::File,
    0x200,
    std::span(null_encryption_keys),
    std::span(fh5_file_decryption_keys).subspan<1, 17>(),
    std::span(fh5_file_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH5,
    KeyType::Profile,
    0x200,
    std::span(fh5_profile_encryption_keys).subspan<1, 17>(),
    std::span(fh5_profile_decryption_keys).subspan<1, 17>(),
    std::span(fh5_profile_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH5,
    KeyType::Photo,
    0x200,
    std::span(fh5_photo_encryption_keys).subspan<1, 17>(),
    std::span(fh5_photo_decryption_keys).subspan<1, 17>(),
    std::span(fh5_photo_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH5,
    KeyType::Dynamic,
    0x200,
    std::span(fh5_dynamic_encryption_keys).subspan<1, 17>(),
    std::span(fh5_dynamic_decryption_keys).subspan<1, 17>(),
    std::span(fh5_dynamic_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH5,
    KeyType::GameDB,
    0x20000,
    std::span(null_encryption_keys),
    std::span(fh5_gamedb_decryption_keys).subspan<1, 17>(),
    std::span(fh5_gamedb_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables,
    std::make_shared<Obfuscation>(fh5_gamedb_obfuscation_seed, fh5_crc32_mapping)
  },
  {
    GameType::FH5,
    KeyType::SFS,
    0x20000,
    std::span(null_encryption_keys),
    std::span(fh5_sfs_decryption_keys).subspan<1, 17>(),
    std::span(fh5_sfs_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH4,
    KeyType::File,
    0x200,
    std::span(null_encryption_keys),
    std::span(fh4_file_decryption_keys).subspan<1, 17>(),
    std::span(fh4_file_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH4,
    KeyType::Profile,
    0x200,
    std::span(fh4_profile_encryption_keys).subspan<1, 17>(),
    std::span(fh4_profile_decryption_keys).subspan<1, 17>(),
    std::span(fh4_profile_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH4,
    KeyType::Photo,
    0x200,
    std::span(fh4_photo_encryption_keys).subspan<1, 17>(),
    std::span(fh4_photo_decryption_keys).subspan<1, 17>(),
    std::span(fh4_photo_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH4,
    KeyType::Dynamic,
    0x200,
    std::span(fh4_dynamic_encryption_keys).subspan<1, 17>(),
    std::span(fh4_dynamic_decryption_keys).subspan<1, 17>(),
    std::span(fh4_dynamic_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH4,
    KeyType::GameDB,
    0x20000,
    std::span(null_encryption_keys),
    std::span(fh4_gamedb_decryption_keys).subspan<1, 17>(),
    std::span(fh4_gamedb_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables,
    std::make_shared<Obfuscation>(fh4_gamedb_obfuscation_seed, fm6apex_crc32_mapping)
  },
  {
    GameType::FH4,
    KeyType::SFS,
    0x20000,
    std::span(null_encryption_keys),
    std::span(fh4_sfs_decryption_keys).subspan<1, 17>(),
    std::span(fh4_sfs_mac_keys).subspan<1, 13>(),
    fh4_encryption_tables,
    fh4_decryption_tables,
    fh4_mac_tables
  },
  {
    GameType::FH3,
    KeyType::File,
    0x200,
    std::span(null_encryption_keys),
    std::span(fh3_file_decryption_keys).subspan<1, 17>(),
    std::span(fh3_file_mac_keys).subspan<1, 13>(),
    fh3_encryption_tables,
    fh3_decryption_tables,
    fh3_mac_tables
  },
  {
    GameType::FH3,
    KeyType::Profile,
    0x200,
    std::span(fh3_profile_encryption_keys).subspan<1, 17>(),
    std::span(fh3_profile_decryption_keys).subspan<1, 17>(),
    std::span(fh3_profile_mac_keys).subspan<1, 13>(),
    fh3_encryption_tables,
    fh3_decryption_tables,
    fh3_mac_tables
  },
  {
    GameType::FH3,
    KeyType::Photo,
    0x200,
    std::span(fh3_photo_encryption_keys).subspan<1, 17>(),
    std::span(fh3_photo_decryption_keys).subspan<1, 17>(),
    std::span(fh3_photo_mac_keys).subspan<1, 13>(),
    fh3_encryption_tables,
    fh3_decryption_tables,
    fh3_mac_tables
  },
  {
    GameType::FH3,
    KeyType::GameDB,
    0x20000,
    std::span(null_encryption_keys),
    std::span(fh3_gamedb_decryption_keys).subspan<1, 17>(),
    std::span(fh3_gamedb_mac_keys).subspan<1, 13>(),
    fh3_encryption_tables,
    fh3_decryption_tables,
    fh3_mac_tables,
    std::make_shared<Obfuscation>(fh3_gamedb_obfuscation_seed, fm6apex_crc32_mapping)
  },
  {
    GameType::FH3,
    KeyType::SFS,
    0x20000,
    std::span(null_encryption_keys),
    std::span(fh3_sfs_decryption_keys).subspan<1, 17>(),
    std::span(fh3_sfs_mac_keys).subspan<1, 13>(),
    fh3_encryption_tables,
    fh3_decryption_tables,
    fh3_mac_tables
  }
};
