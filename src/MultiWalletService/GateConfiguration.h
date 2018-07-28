// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <stdexcept>
#include <cstdint>

#include <boost/program_options.hpp>

namespace MultiWalletService
{

class ConfigurationError : public std::runtime_error
{
public:
  ConfigurationError(const char *desc) : std::runtime_error(desc) {}
};

struct Configuration
{
  Configuration();

  void init(const boost::program_options::variables_map &options);
  static void initOptions(boost::program_options::options_description &desc);

  std::string bindAddress;
  uint16_t bindPort;

  std::string logFile;
  std::string serverRoot;

  bool daemonize;
  bool testnet;

  size_t logLevel;
};

} //namespace MultiWalletService
