// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "version.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "DaemonCommandsHandler.h"

#include "common/SignalHandler.h"
#include "crypto/hash.h"
#include "cryptonote/core/Core.h"
#include "command_line/daemon.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/Currency.h"
#include "cryptonote/core/MinerConfig.h"
#include "cryptonote/protocol/handler.h"
#include "p2p/NetNode.h"
#include "p2p/NetNodeConfig.h"
#include "rpc/RpcServer.h"
#include "rpc/RpcServerConfig.h"
#include "version.h"

#include "logging/ConsoleLogger.h"
#include <logging/LoggerManager.h>

#if defined(WIN32)
#include <crtdbg.h>
#endif

using Common::JsonValue;
using namespace cryptonote;
using namespace Logging;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

// bool command_line_preprocessor(const boost::program_options::variables_map& vm, LoggerRef& logger);

JsonValue buildLoggerConfiguration(Level level, const std::string& logfile) {
  JsonValue loggerConfiguration(JsonValue::OBJECT);
  loggerConfiguration.insert("globalLevel", static_cast<int64_t>(level));

  JsonValue& cfgLoggers = loggerConfiguration.insert("loggers", JsonValue::ARRAY);

  JsonValue& fileLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
  fileLogger.insert("type", "file");
  fileLogger.insert("filename", logfile);
  fileLogger.insert("level", static_cast<int64_t>(TRACE));

  JsonValue& consoleLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
  consoleLogger.insert("type", "console");
  consoleLogger.insert("level", static_cast<int64_t>(TRACE));
  consoleLogger.insert("pattern", "%T %L ");

  return loggerConfiguration;
}


int main(int argc, char* argv[])
{

#ifdef WIN32
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

  LoggerManager logManager;
  LoggerRef logger(logManager, "daemon");

  try {

    command_line::Daemon parse = command_line::Daemon();

    po::options_description &desc_cmd_sett = parse.desc_cmd_sett;
    po::options_description &desc_cmd_only = parse.desc_cmd_only;
    po::variables_map &vm = parse.vm;

    RpcServerConfig::initOptions(desc_cmd_sett);
    NetNodeConfig::initOptions(desc_cmd_sett);
    MinerConfig::initOptions(desc_cmd_sett);

    bool r = parse.parse(argc, argv, [&]() {
    });

    if (!r)
      return 1;
  
    auto modulePath = boost::filesystem::path(argv[0]);
    auto cfgLogFile = boost::filesystem::path(parse.getLogFile());

    if (cfgLogFile.empty()) {
      cfgLogFile = fs::change_extension(modulePath, ".log").string();
    } else {
      if (!cfgLogFile.parent_path().empty()) {
        cfgLogFile = modulePath.root_path().string() + cfgLogFile.string();
      }
    }

    Level cfgLogLevel = static_cast<Level>(static_cast<int>(Logging::ERROR) + 
      parse.getLogLevel());

    // configure logging
    logManager.configure(buildLoggerConfiguration(cfgLogLevel, cfgLogFile.string()));

    logger(INFO) << cryptonote::CRYPTONOTE_NAME << " v" << PROJECT_VERSION_LONG;

    if (parse.checkVersion()) {
      return 0;
    }

    logger(INFO) << "Module folder: " << argv[0];
    bool testnet_mode = parse.getTestMode();
    if (testnet_mode)
    {
        logger(INFO) << "Starting in testnet mode!" << std::endl;
    }
  
    //create objects and link them
    cryptonote::CurrencyBuilder currencyBuilder(logManager, os::appdata::path());
    // currencyBuilder.testnet(testnet_mode);

    try {
      currencyBuilder.currency();
    } catch (std::exception&) {
      std::cout << "GENESIS_COINBASE_TX_HEX constant has an incorrect value. Please launch: " << cryptonote::CRYPTONOTE_NAME << "d --" << parse.getGenesisName();
      return 1;
    }

    cryptonote::Currency currency = currencyBuilder.currency();
    cryptonote::core ccore(currency, nullptr, logManager);

    cryptonote::Checkpoints checkpoints(logManager);
    for (const auto& cp : cryptonote::CHECKPOINTS) {
      checkpoints.add(cp.height, cp.blockId);
    }

    if (!testnet_mode) {
      ccore.set_checkpoints(std::move(checkpoints));
    }

    parse.assign();
    NetNodeConfig netNodeConfig;
    netNodeConfig.init(vm);
    netNodeConfig.setTestnet(testnet_mode);
    MinerConfig minerConfig;
    minerConfig.init(vm);
    RpcServerConfig rpcConfig;
    rpcConfig.init(vm);

    if (!parse.configFolderDefaulted) {
      boost::filesystem::path path(parse.configFolder);

      if (!boost::filesystem::exists(parse.configFolder)) {
        throw std::runtime_error("Directory does not exist: " + parse.configFolder);
      }
    } else {
      boost::filesystem::path path(parse.configFolder);
      bool exists = boost::filesystem::exists(path) ? true : boost::filesystem::create_directory(path);
      if (!exists) {
        throw std::runtime_error("Can't create directory: " + parse.configFolder);
      }
    }

    System::Dispatcher dispatcher;

    cryptonote::CryptoNoteProtocolHandler cprotocol(currency, dispatcher, ccore, nullptr, logManager);
    cryptonote::NodeServer p2psrv(dispatcher, cprotocol, logManager);
    cryptonote::RpcServer rpcServer(dispatcher, logManager, ccore, p2psrv, cprotocol);

    cprotocol.set_p2p_endpoint(&p2psrv);
    ccore.set_cryptonote_protocol(&cprotocol);
    DaemonCommandsHandler dch(ccore, p2psrv, logManager);

    // initialize objects
    logger(INFO) << "Initializing p2p server...";
    if (!p2psrv.init(netNodeConfig)) {
      logger(ERROR, BRIGHT_RED) << "Failed to initialize p2p server.";
      return 1;
    }
    logger(INFO) << "P2p server initialized OK";

    //logger(INFO) << "Initializing core rpc server...";
    //if (!rpc_server.init(vm)) {
    //  logger(ERROR, BRIGHT_RED) << "Failed to initialize core rpc server.";
    //  return 1;
    //}
    // logger(INFO, BRIGHT_GREEN) << "Core rpc server initialized OK on port: " << rpc_server.get_binded_port();

    // initialize core here
    logger(INFO) << "Initializing core...";
    if (!ccore.init(minerConfig, true)) {
      logger(ERROR, BRIGHT_RED) << "Failed to initialize core";
      return 1;
    }
    logger(INFO) << "Core initialized OK";

    // start components
    if (!parse.getConsole()) {
      dch.start_handling();
    }

    logger(INFO) << "Starting core rpc server on address " << rpcConfig.getBindAddress();
    rpcServer.start(rpcConfig.bindIp, rpcConfig.bindPort);
    logger(INFO) << "Core rpc server started ok";

    Tools::SignalHandler::install([&dch, &p2psrv] {
      dch.stop_handling();
      p2psrv.sendStopSignal();
    });

    logger(INFO) << "Starting p2p net loop...";
    p2psrv.run();
    logger(INFO) << "p2p net loop stopped";

    dch.stop_handling();

    //stop components
    logger(INFO) << "Stopping core rpc server...";
    rpcServer.stop();

    //deinitialize components
    logger(INFO) << "Deinitializing core...";
    ccore.deinit();
    logger(INFO) << "Deinitializing p2p...";
    p2psrv.deinit();

    ccore.set_cryptonote_protocol(NULL);
    cprotocol.set_p2p_endpoint(NULL);

  } catch (const std::exception& e) {
    logger(ERROR, BRIGHT_RED) << "Exception: " << e.what();
    return 1;
  }

  logger(INFO) << "Node stopped.";
  return 0;
}
