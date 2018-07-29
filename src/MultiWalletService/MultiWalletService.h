// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <fstream>

#include "ConfigurationManager.h"
#include "GateConfiguration.h"

#include "Logging/ConsoleLogger.h"
#include "Logging/LoggerGroup.h"
#include "Logging/StreamLogger.h"

#include "PaymentGate/NodeFactory.h"

#include "CryptoNoteCore/Currency.h"
#include "System/Event.h"

using namespace std;

namespace MultiWalletService
{
class MultiWallet
{
public:
  MultiWallet() : dispatcher(nullptr), stopEvent(nullptr), config(), logger(), currencyBuilder(logger)
  {
  }

  bool init(int argc, char **argv);

  const MultiWalletService::ConfigurationManager &getConfig() const { return config; }
  const CryptoNote::Currency getCurrency();

  void run();
  void stop();

  Logging::ILogger &getLogger() { return logger; }

private:
  void runRpcProxy(Logging::LoggerRef &log);

  void runWalletService(const CryptoNote::Currency &currency, CryptoNote::INode &node);

  System::Dispatcher *dispatcher;
  System::Event *stopEvent;
  MultiWalletService::ConfigurationManager config;
  CryptoNote::CurrencyBuilder currencyBuilder;

  Logging::LoggerGroup logger;
  ofstream fileStream;
  Logging::StreamLogger fileLogger;
  Logging::ConsoleLogger consoleLogger;
};
} // namespace MultiWalletService
