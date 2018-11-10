// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "command_line/ConfigurationManager.h"
// #include "command_line/PaymentServiceConfiguration.h"

#include "logging/ConsoleLogger.h"
#include "logging/LoggerGroup.h"
#include "logging/StreamLogger.h"

#include "payment_gate/NodeFactory.h"
#include "payment_gate/WalletService.h"
#include "common/os.h"

class PaymentGateService {
public:
  PaymentGateService() : dispatcher(nullptr), stopEvent(nullptr), config(), service(nullptr), logger(), currencyBuilder(
    os::appdata::path(),
    config::mainnet::data,
    logger
    ) {
  }

  bool init(int argc, char** argv);

  const PaymentService::ConfigurationManager& getConfig() const { return config; }
  PaymentService::WalletConfiguration getWalletConfig() const;
  const cryptonote::Currency getCurrency();

  void run();
  void stop();
  
  Logging::ILogger& getLogger() { return logger; }

private:

  void runInProcess(Logging::LoggerRef& log);
  void runRpcProxy(Logging::LoggerRef& log);

  void runWalletService(const cryptonote::Currency& currency, cryptonote::INode& node);

  System::Dispatcher* dispatcher;
  System::Event* stopEvent;
  PaymentService::ConfigurationManager config;
  PaymentService::WalletService* service;
  cryptonote::CurrencyBuilder currencyBuilder;
  
  Logging::LoggerGroup logger;
  std::ofstream fileStream;
  Logging::StreamLogger fileLogger;
  Logging::ConsoleLogger consoleLogger;
};
