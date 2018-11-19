// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MultiWalletService.h"

#include <future>

#include "common/SignalHandler.h"
#include "InProcessNode/InProcessNode.h"
#include "logging/LoggerRef.h"
#include "MultiServiceJsonRpcServer.h"

#include "command_line/CoreConfig.h"
#include "cryptonote/core/core.h"
#include "cryptonote/protocol/handler.h"
#include "p2p/NetNode.h"
#include "payment_gate/WalletFactory.h"
#include <system/Context.h>
#include "NodeRpcProxy/NodeRpcProxy.h"

#ifdef ERROR
#undef ERROR
#endif

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

using namespace PaymentService;

namespace MultiWalletService
{

void changeDirectory(const std::string &path)
{
  if (chdir(path.c_str()))
  {
    throw std::runtime_error("Couldn't change directory to \'" + path + "\': " + strerror(errno));
  }
}

void stopSignalHandler(MultiWallet *pg)
{
  pg->stop();
}

bool MultiWallet::init(int argc, char **argv)
{
  if (!config.init(argc, argv))
  {
    return false;
  }

  logger.setMaxLevel(static_cast<Logging::Level>(config.gateConfiguration.logLevel));
  logger.addLogger(consoleLogger);

  Logging::LoggerRef log(logger, "main");

  if (config.gateConfiguration.testnet)
  {
    log(Logging::INFO) << "Starting in testnet mode";
    currencyBuilder.testnet(true);
  }

  if (!config.gateConfiguration.serverRoot.empty())
  {
    changeDirectory(config.gateConfiguration.serverRoot);
    log(Logging::INFO) << "Current working directory now is " << config.gateConfiguration.serverRoot;
  }

  fileStream.open(config.gateConfiguration.logFile, std::ofstream::app);

  if (!fileStream)
  {
    throw std::runtime_error("Couldn't open log file");
  }

  fileLogger.attachToStream(fileStream);
  logger.addLogger(fileLogger);

  return true;
}

const cryptonote::Currency MultiWallet::getCurrency()
{
  return currencyBuilder.currency();
}

void MultiWallet::run()
{

  System::Dispatcher localDispatcher;
  System::Event localStopEvent(localDispatcher);

  this->dispatcher = &localDispatcher;
  this->stopEvent = &localStopEvent;

  Tools::SignalHandler::install(std::bind(&stopSignalHandler, this));

  Logging::LoggerRef log(logger, "run");

  runRpcProxy(log);

  this->dispatcher = nullptr;
  this->stopEvent = nullptr;
}

void MultiWallet::stop()
{
  Logging::LoggerRef log(logger, "stop");

  log(Logging::INFO) << "Stop signal caught";

  if (dispatcher != nullptr)
  {
    dispatcher->remoteSpawn([&]() {
      if (stopEvent != nullptr)
      {
        stopEvent->set();
      }
    });
  }
}

void MultiWallet::runRpcProxy(Logging::LoggerRef &log)
{
  log(Logging::INFO) << "Starting Payment Gate with remote node";
  cryptonote::Currency currency = currencyBuilder.currency();

  log(Logging::INFO) << "daemon host " << config.remoteNodeConfig.daemonHost;
  log(Logging::INFO) << "daemon port " << config.remoteNodeConfig.daemonPort;

  std::unique_ptr<INode> node(new NodeRpcProxy(
      config.remoteNodeConfig.daemonHost,
      config.remoteNodeConfig.daemonPort));

  // std::unique_ptr<cryptonote::INode> node(
  //     PaymentService::NodeFactory::createNode(
  //         config.remoteNodeConfig.daemonHost,
  //         config.remoteNodeConfig.daemonPort));

  // std::unique_ptr<INode> node(new NodeRpcProxy(daemon_host, daemon_port));

  std::promise<std::error_code> errorPromise;
  std::future<std::error_code> error = errorPromise.get_future();
  auto callback = [&errorPromise](std::error_code e) { errorPromise.set_value(e); };
  node->init(callback);
  if (error.get())
  {
    log(Logging::ERROR) << ("failed to init NodeRPCProxy");
    return;
  }

  log(Logging::ERROR) << ("NodeRPCProxy started");

  runWalletService(currency, *node, log);

  log(Logging::INFO) << "Started wallet Service";
}

void MultiWallet::runWalletService(const cryptonote::Currency &currency, cryptonote::INode &node, Logging::LoggerRef &log)
{

  WalletInterface *wallet = new WalletInterface(*dispatcher, currency, node, logger);

  dispatcher->remoteSpawn([this, &log, &currency, &node, &wallet]() {
    log(Logging::INFO) << "starting wallet";

    // wallet->init();

    log(Logging::INFO) << "end starting wallet";
  });

  log(Logging::INFO) << "starting rpc server";

  MultiWalletService::MultiServiceJsonRpcServer rpcServer(*dispatcher, *stopEvent, log.getLogger(), *wallet);
  rpcServer.start(config.gateConfiguration.bindAddress, config.gateConfiguration.bindPort);
  log(Logging::INFO) << "ending rpc server";

  delete wallet;
}

} // namespace MultiWalletService