#include <boost/program_options.hpp>
#include "contexts/contexts.h"

namespace po = boost::program_options;

std::map<KeyType, Name> key_types = {
  {
    KeyType::Profile,
    {"Profile", "Profile"}
  },
  {
    KeyType::Reward,
    {"ForzaProfileProdContainer.LevelRewardCache", "Reward"}
  },
  {
    KeyType::GameDB,
    {"GameDB", "GameDB"}
  },
  {
    KeyType::ConfigFile,
    {"ConfigFile", "ConfigFile"}
  },
  {
    KeyType::File,
    {"File", "File"}
  },
  {
    KeyType::SFS,
    {"SFS", "SFS"}
  },
  {
    KeyType::Photo,
    {"Photo", "Photo"}
  },
  {
    KeyType::Dynamic,
    {"Dynamic", "Dynamic"}
  },
  {
    KeyType::Telemetry,
    {"Telemetry", "Telemetry"}
  }
};

std::map<GameType, Name> game_types = {
  {
    GameType::FH5,
    {"Forza Horizon 5", "FH5"}
  },
  {
    GameType::FH4,
    {"Forza Horizon 4", "FH4"}
  },
  {
    GameType::FM7,
    {"Forza Motorsport 7", "FM7"}
  },
  {
    GameType::FH3,
    {"Forza Horizon 3", "FH3"}
  },
  {
    GameType::FM6Apex,
    {"Forza Motorsport 6: Apex", "FM6Apex"}
  }
};

void validate(boost::any &v, const std::vector<std::string> &values, GameType *target_type, int) {
  po::validators::check_first_occurrence(v);

  const std::string &s = po::get_single_string(values);

  auto result = std::find_if(std::begin(game_types), std::end(game_types), [&](auto &v) {
    return v.second.abbreviation == s;
  });

  if (result != std::end(game_types)) {
    v = boost::any(result->first);
  } else {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
}

void validate(boost::any &v, const std::vector<std::string> &values, KeyType *target_type, int) {
  po::validators::check_first_occurrence(v);

  const std::string &s = po::get_single_string(values);

  auto result = std::find_if(std::begin(key_types), std::end(key_types), [&](auto &v) {
    return v.second.abbreviation == s;
  });

  if (result != std::end(key_types)) {
    v = boost::any(result->first);
  } else {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
}
