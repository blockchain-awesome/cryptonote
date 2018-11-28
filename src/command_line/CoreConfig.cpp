// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/filesystem.hpp>
#include "CoreConfig.h"

#include "common/os.h"
#include "command_line/options.h"

namespace cryptonote
{

CoreConfig::CoreConfig() : configFolder(os::appdata::path())
{
}

void CoreConfig::init(const boost::program_options::variables_map &options)
{
  if (options.count(command_line::arg_data_dir.name) != 0 && (!options[command_line::arg_data_dir.name].defaulted() || configFolder == os::appdata::path()))
  {
    configFolder = command_line::get_arg(options, command_line::arg_data_dir);
    configFolderDefaulted = options[command_line::arg_data_dir.name].defaulted();
    if (configFolderDefaulted) {
      configFolder = os::appdata::path();
    }
  }
}

void CoreConfig::checkDataDir()
{
  std::cout << "Checking Data Dir :" << configFolder << std::endl;
  boost::filesystem::path path(configFolder);
  if (!boost::filesystem::exists(path))
  {
    std::cout << "Dir not found!" << std::endl;

    if (!configFolderDefaulted)
    {
      throw std::runtime_error("Directory does not exist: " + configFolder);
    }
    else
    {
      std::cout << "Dir defaulted!" << std::endl;

      if (!boost::filesystem::create_directory(path))
      {
        throw std::runtime_error("Can't create directory: " + configFolder);
      }
    }
  }
}

std::string &CoreConfig::getDir() {
    if (configFolderDefaulted) {
      configFolder = os::appdata::path();
    }
    return configFolder;
  }


void CoreConfig::initOptions(boost::program_options::options_description &desc)
{
}
} //namespace cryptonote
