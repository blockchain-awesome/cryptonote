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
#include "cryptonote/core/CoreConfig.h"
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
using namespace command_line;
using namespace cryptonote;
using namespace Logging;

namespace fs = boost::filesystem;

void print_genesis_tx_hex() {
  Logging::ConsoleLogger logger;
  cryptonote::Transaction tx = cryptonote::CurrencyBuilder(logger, os::appdata::path()).generateGenesisTransaction();
  cryptonote::BinaryArray txb = cryptonote::toBinaryArray(tx);
  std::string tx_hex = Common::toHex(txb);

  std::cout << "Insert this line into your coin configuration file as is: " << std::endl;
  std::cout << "const char GENESIS_COINBASE_TX_HEX[] = \"" << tx_hex << "\";" << std::endl;

  return;
}

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

  OptionsNames names;
  names.command = "Command line options";
  names.setting = "Command line options and settings options";
  names.full = "Allowed options";
  Daemon cli(names);

  try {
    cli.init();

    po::options_description& desc_cmd_sett = cli.desc_cmd_sett;
    RpcServerConfig::initOptions(desc_cmd_sett);
    CoreConfig::initOptions(desc_cmd_sett);
    NetNodeConfig::initOptions(desc_cmd_sett);
    MinerConfig::initOptions(desc_cmd_sett);

    cli.setup();

    po::variables_map &vm = cli.vm;
    bool r = cli.parse(argc, argv, [&]()
    {
      if (get_arg(vm, arg_print_genesis_tx)) {
        print_genesis_tx_hex();
        return false;
      }

      std::string data_dir = get_arg(vm, arg_data_dir);
      std::string config = get_arg(vm, arg_config_file);

      boost::filesystem::path data_dir_path(data_dir);
      boost::filesystem::path config_path(config);
      if (!config_path.has_parent_path()) {
        config_path = data_dir_path / config_path;
      }

      boost::system::error_code ec;
      if (boost::filesystem::exists(config_path, ec)) {
        po::store(po::parse_config_file<char>(config_path.string<std::string>().c_str(), desc_cmd_sett), vm);
      }
      po::notify(vm);
      return true;
    });

    if (!r)
      return 1;
  
    auto modulePath = boost::filesystem::path(argv[0]);
    auto cfgLogFile = boost::filesystem::path(get_arg(vm, arg_log_file));

    if (cfgLogFile.empty()) {
      cfgLogFile = fs::change_extension(modulePath, ".log").string();
    } else {
      if (!cfgLogFile.parent_path().empty()) {
        cfgLogFile = modulePath.root_path().string() + cfgLogFile.string();
      }
    }

    Level cfgLogLevel = static_cast<Level>(static_cast<int>(Logging::ERROR) + get_arg(vm, arg_log_level));

    // configure logging
    logManager.configure(buildLoggerConfiguration(cfgLogLevel, cfgLogFile.string()));

    logger(INFO) << cryptonote::CRYPTONOTE_NAME << " v" << PROJECT_VERSION_LONG;

    if (cli.checkVersion()) {
      return 0;
    }

    logger(INFO) << "Module folder: " << argv[0];

    bool testnet_mode = get_arg(vm, arg_testnet_on);
    if (testnet_mode) {
      logger(INFO) << "Starting in testnet mode!";
    }

    //create objects and link them
    cryptonote::CurrencyBuilder currencyBuilder(logManager, os::appdata::path());
    // currencyBuilder.testnet(testnet_mode);

    try {
      currencyBuilder.currency();
    } catch (std::exception&) {
      std::cout << "GENESIS_COINBASE_TX_HEX constant has an incorrect value. Please launch: " << cryptonote::CRYPTONOTE_NAME << "d --" << arg_print_genesis_tx.name;
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

    CoreConfig coreConfig;
    coreConfig.init(vm);
    NetNodeConfig netNodeConfig;
    netNodeConfig.init(vm);
    netNodeConfig.setTestnet(testnet_mode);
    MinerConfig minerConfig;
    minerConfig.init(vm);
    RpcServerConfig rpcConfig;
    rpcConfig.init(vm);

    if (!coreConfig.configFolderDefaulted) {
      boost::filesystem::path path(coreConfig.configFolder);

      if (!boost::filesystem::exists(coreConfig.configFolder)) {
        throw std::runtime_error("Directory does not exist: " + coreConfig.configFolder);
      }
    } else {
      boost::filesystem::path path(coreConfig.configFolder);
      bool exists = boost::filesystem::exists(path) ? true : boost::filesystem::create_directory(path);
      if (!exists) {
        throw std::runtime_error("Can't create directory: " + coreConfig.configFolder);
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
    if (!ccore.init(coreConfig, minerConfig, true)) {
      logger(ERROR, BRIGHT_RED) << "Failed to initialize core";
      return 1;
    }
    logger(INFO) << "Core initialized OK";

    // start components
    if (!has_arg(vm, arg_console)) {
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

