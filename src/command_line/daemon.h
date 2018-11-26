// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <boost/program_options.hpp>
#include "options.h"
#include "config/common.h"

namespace command_line
{

namespace po = boost::program_options;
namespace fs = boost::filesystem;
class Daemon : public CommandParser
{
public:
  Daemon(OptionsNames &names, config::config_t &config) : CommandParser(names), m_config(config){};
  virtual bool innerParse();

  void init();
  bool checkVersion();
  void printGenesisTx();

  bool parseHelp();
  void parseConfigFile();

  fs::path getLogFile();

  void setConfig(config::config_t &config) {
    m_config = config;
  };
  config::config_t &m_config;
};

} // namespace command_line
