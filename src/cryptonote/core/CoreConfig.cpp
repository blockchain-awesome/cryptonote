// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoreConfig.h"

#include "common/os.h"
#include "command_line/options.h"

namespace cryptonote {

CoreConfig::CoreConfig() {
  configFolder = os::appdata::path();
}

void CoreConfig::init(const boost::program_options::variables_map& options) {
  if (options.count(command_line::arg_data_dir.name) != 0 && (!options[command_line::arg_data_dir.name].defaulted() || configFolder == os::appdata::path())) {
    configFolder = command_line::get_arg(options, command_line::arg_data_dir);
    configFolderDefaulted = options[command_line::arg_data_dir.name].defaulted();
  }
}

void CoreConfig::initOptions(boost::program_options::options_description& desc) {
}
} //namespace cryptonote
