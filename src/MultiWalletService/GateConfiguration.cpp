// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GateConfiguration.h"

#include <iostream>
#include <algorithm>
#include <boost/program_options.hpp>

#include "logging/ILogger.h"
#include "CryptoNoteConfig.h"

namespace po = boost::program_options;

using namespace cryptonote;

namespace MultiWalletService {

Configuration::Configuration(): logFile("multi_wallet_gate.log"), bindAddress("") {
  daemonize = false;
  logFile = ;
  testnet = false;
  logLevel = Logging::INFO;
  bindPort = 0;
}

void Configuration::initOptions(boost::program_options::options_description& desc) {
  desc.add_options()
      ("bind-address", po::value<std::string>()->default_value("0.0.0.0"), "multi wallet service bind address")
      ("bind-port", po::value<uint16_t>()->default_value(RPC_WALLET_PORT), "multi service bind port")
      ("daemon,d", "run as daemon in Unix or as service in Windows")
      ("log-file,l", po::value<std::string>(), "log file")
      ("server-root", po::value<std::string>(), "server root. The service will use it as working directory. Don't set it if don't want to change it")
      ("log-level", po::value<size_t>(), "log level");
}

void Configuration::init(const boost::program_options::variables_map& options) {
  if (options.count("daemon") != 0) {
    daemonize = true;
  }

  if (options["testnet"].as<bool>()) {
    testnet = true;
  }

  if (options.count("log-file") != 0) {
    logFile = options["log-file"].as<std::string>();
  }

  if (options.count("log-level") != 0) {
    logLevel = options["log-level"].as<size_t>();
    if (logLevel > Logging::TRACE) {
      std::string error = "log-level option must be in " + std::to_string(Logging::FATAL) +  ".." + std::to_string(Logging::TRACE) + " interval";
      throw ConfigurationError(error.c_str());
    }
  }

  if (options.count("server-root") != 0) {
    serverRoot = options["server-root"].as<std::string>();
  }

  if (options.count("bind-address") != 0 && (!options["bind-address"].defaulted() || bindAddress.empty())) {
    bindAddress = options["bind-address"].as<std::string>();
  }

  if (options.count("bind-port") != 0 && (!options["bind-port"].defaulted() || bindPort == 0)) {
    bindPort = options["bind-port"].as<uint16_t>();
  }
}

} //namespace MultiWalletService
