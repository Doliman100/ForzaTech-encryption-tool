#include <iostream>
#include <format>
#include <fstream>
#include <filesystem>
#include <ranges>
#include <fcntl.h>
#include <io.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/hex.hpp>
#include "streams/fh3.h"
#include "streams/fm6apex.h"
#include "tfit/encryption.h"
#include "iobinary.h"
#include "zip.h"
#include "contexts/contexts.h"

// requires: boost 1.82.0 https://boostorg.jfrog.io/artifactory/main/release/1.82.0/binaries/

// TFIT
//   Encryption
//   Decryption
// Obfuscation
// Zip
// Stream
//   Encryption
//   Decryption
// FileFormat
//   FH3
//   FM6Apex

namespace po = boost::program_options;

uint32_t verbose = 1;

struct IV {
  IV() {

  }

  IV(const std::string &str) {
    std::string str_tmp(str);
    str_tmp.erase(std::remove_if(str_tmp.begin(), str_tmp.end(), [](unsigned char c) { return std::isspace(c); }), str_tmp.end());
    auto result = boost::algorithm::unhex(str_tmp, std::begin(data));
    if (result != std::end(data)) {
      throw std::runtime_error("IV length is less than 16 bytes.");
    }
  }

  std::array<uint8_t, 16> data = {};
};

void validate(boost::any &v, const std::vector<std::string> &values, IV *target_type, int) {
  po::validators::check_first_occurrence(v);

  const std::string &s = po::get_single_string(values);

  try {
    v = boost::any(IV(s));
  } catch (std::exception &) {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
}


void PrintHelp(po::options_description &options) {
  std::cout << options;
  std::cout << std::endl;
  std::cout << "Example:" << std::endl;
  std::cout << "  .\\EncryptionTool.exe -i\"C:\\Program Files (x86)\\DODI-Repacks\\Forza Horizon 5\\media\\Physics\\PI.xml\" -o\"PI.xml\"" << std::endl;
  std::cout << "  .\\EncryptionTool.exe -m0 -gFH5 -kProfile --iv=\"0C CF 15 0C A7 23 A0 23 7A A2 45 63 38 E0 4A 0C\" -i\"User_69C2EF99.ProfileData\" -o\"C:\\Users\\Public\\Documents\\EMPRESS\\1551360\\remote\\1551360\\remote\\1774383001\\User_69C2EF99.ProfileData\"" << std::endl;
}

std::unique_ptr<DecryptionStream> CreateEncryptedFile(std::istream &input, uint32_t input_size) {
  // MAC validation
  std::unique_ptr<DecryptionStream> encrypted_file = std::make_unique<ForzaHorizon3::DecryptionStream>(input, input_size);
  if (!*encrypted_file) {
    encrypted_file = std::make_unique<ForzaMotorsport6Apex::DecryptionStream>(input, input_size);
  }
  if (!*encrypted_file) {
    std::cerr << "Error: None of the keys matched." << std::endl;
    throw "error1";
  }
  if (verbose > 0) {
    std::cout << "Game: " << encrypted_file->game_type() << std::endl;
    std::cout << "Key: " << encrypted_file->key_type() << std::endl;
    std::cout << "Data block size: " << std::format("0x{:X}", encrypted_file->data_block_size()) << std::endl;
    std::cout << "Padding size: " << std::format("0x{:X}", encrypted_file->padding_size()) << std::endl;
    std::cout << "Data size: " << std::format("0x{:X}", encrypted_file->data_size()) << std::endl;
    PrintBlock(std::cout << "IV: ", encrypted_file->iv()) << std::endl;
  }
  return encrypted_file;
}

int main(int argc, char *argv[]) {
  _setmode(_fileno(stdout), _O_BINARY); // https://stackoverflow.com/q/16888339

  // options
  po::options_description all_options;

  po::options_description options("Options");
  options.add_options()
    ("help,h", "Print this help message.")
    ("verbose,v", po::value<uint32_t>(&verbose)->default_value(1)->value_name("number"), "Set verbosity level. (0 - no output, 1 - default, 2 - verbose)")
    ("mode,m", po::value<uint32_t>()->default_value(1)->value_name("number")->notifier([](uint32_t v) {
        if (v > 1) {
          throw po::validation_error(po::validation_error::invalid_option_value, "mode", std::to_string(v), 1);
        }
      })->required(), "(0 - encryption, 1 - decryption)") // 2 - key extraction
    ("input,i", po::value<std::string>()->value_name("string")->required(), "Set the input file path.")
    ("output,o", po::value<std::string>()->value_name("string")->required(), "Set the output file path. ('-' - stdout)")
    ("yes,y", "Overwrite output file, if exists.");
  all_options.add(options);
  po::options_description encryption_options("Encryption options");
  encryption_options.add_options()
    ("game,g", po::value<GameType>()->value_name("string")->required(), "Game title. (FM6Apex, FH3, FM7, FH4, FH5)")
    ("key,k", po::value<KeyType>()->value_name("string")->required(), "Key type. (SFS, GameDB, File, ConfigFile, Profile, Reward, Photo, Dynamic)")
    ("iv", po::value<IV>()->default_value(IV(), "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00")->value_name("string"), "Initialization vector. Hex string of length 32.");
  all_options.add(encryption_options);
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(options).allow_unregistered().run(), vm);
    if (vm.count("help")) {
      PrintHelp(all_options);
      return 1;
    }
    if (vm["mode"].as<uint32_t>() == 0) {
      options.add(encryption_options);
      po::store(po::parse_command_line(argc, argv, options), vm);
    }
    po::notify(vm);
  } catch (boost::program_options::error_with_option_name &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::cout << std::endl;
    PrintHelp(all_options); // do print to stderr in this case?
    return -1;
  }

  // output
  std::string output_path_string = vm["output"].as<std::string>();
  std::shared_ptr<std::ostream> output;
  if (output_path_string == "-") {
    output = std::shared_ptr<std::ostream>(&std::cout, [](void *) {});
  } else {
    std::filesystem::path output_path(output_path_string);
    if (verbose > 1) {
      std::cout << "Output path: " << output_path << std::endl;
    }
    if (vm.count("yes") == 0 && std::filesystem::exists(output_path)) {
      std::cerr << "Error: The output file is already exist." << std::endl;
      return -1;
    }
    output = std::make_shared<std::ofstream>(output_path, std::ios_base::binary);
    if (!*output) {
      std::cerr << "Error: Couldn't open output file." << std::endl;
      return -1;
    }
  }

  // input
  std::filesystem::path input_path(vm["input"].as<std::string>());
  if (verbose > 1) {
    std::cout << "Input path: " << input_path << std::endl;
  }
  if (!std::filesystem::exists(input_path)) {
    std::cerr << "Error: The input file doesn't exist." << std::endl;
    return -1;
  }
  std::ifstream input(input_path, std::ios_base::binary);
  if (!input) {
    std::cerr << "Error: Couldn't open input file." << std::endl;
    return -1;
  }

  if (vm["mode"].as<uint32_t>() == 0) {
    const GameType &game_type = vm["game"].as<GameType>();
    const KeyType &key_type = vm["key"].as<KeyType>();
    const IV &iv = vm["iv"].as<IV>();

    uint32_t input_size = static_cast<uint32_t>(std::filesystem::file_size(input_path));
    std::unique_ptr<EncryptionStream> es;

    auto context = std::find_if(std::begin(ForzaHorizon3::contexts), std::end(ForzaHorizon3::contexts), [&](auto &v) {
      return v.game_type == game_type && v.key_type == key_type;
    });
    if (context == std::end(ForzaHorizon3::contexts)) {
      context = std::find_if(std::begin(ForzaMotorsport6Apex::contexts), std::end(ForzaMotorsport6Apex::contexts), [&](auto &v) {
        return v.game_type == game_type && v.key_type == key_type;
      });
      if (context == std::end(ForzaMotorsport6Apex::contexts)) {
        std::cerr << "Error: The game has no this key type." << std::endl;
        return -1;
      } else {
        es = std::make_unique<ForzaMotorsport6Apex::EncryptionStream>(*output, input_size, iv.data, *context);
      }
    } else {
      es = std::make_unique<ForzaHorizon3::EncryptionStream>(*output, input_size, iv.data, *context);
    }
    if (context->encryption_keys.data() == null_encryption_keys.data()) {
      std::cerr << "Error: The game has no encryption key of this type." << std::endl;
      return -1;
    }

    es->WriteData(input);
  } else {
    try {
      Zip::Transfer zip_reader(input, *output);
      zip_reader.Run([&](Zip::LocalFileHeader &header) -> bool {
        if (header.compression_method() != 22) {
          return false;
        }
        if (verbose > 0) {
          std::cout << "File name: " << header.file_name() << std::endl;
        }
        uint32_t input_size = header.compressed_size();
        std::unique_ptr<DecryptionStream> encrypted_file = CreateEncryptedFile(input, input_size);
        if (!*encrypted_file) {
          return false;
        }
        header.compression_method(8);
        header.compressed_size(encrypted_file->decrypted_file_size());
        header.Write(*output);
        encrypted_file->ReadData(*output);
        std::cout << std::endl;
        return true;
      });
    } catch (Zip::Exception &) {
      uint32_t input_size = static_cast<uint32_t>(std::filesystem::file_size(input_path));
      input.seekg(0);
      std::unique_ptr<DecryptionStream> encrypted_file = CreateEncryptedFile(input, input_size);
      encrypted_file->ReadData(*output);
    }
  }

  return 0;
}
