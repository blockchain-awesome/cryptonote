#include "simplewallet.h"
#include "options.h"
#include "version.h"
#include "wallet/WalletRpcServer.h"
#include <logging/LoggerManager.h>
#include "NodeRpcProxy/NodeRpcProxy.h"
#include "common/SignalHandler.h"

using namespace Common;
using namespace Logging;
using namespace cryptonote;

namespace po = boost::program_options;

JsonValue buildLoggerConfiguration(Level level, const std::string &logfile)
{
  JsonValue loggerConfiguration(JsonValue::OBJECT);
  loggerConfiguration.insert("globalLevel", static_cast<int64_t>(level));

  JsonValue &cfgLoggers = loggerConfiguration.insert("loggers", JsonValue::ARRAY);

  JsonValue &consoleLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
  consoleLogger.insert("type", "console");
  consoleLogger.insert("level", static_cast<int64_t>(TRACE));
  consoleLogger.insert("pattern", "");

  JsonValue &fileLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
  fileLogger.insert("type", "file");
  fileLogger.insert("filename", logfile);
  fileLogger.insert("level", static_cast<int64_t>(TRACE));

  return loggerConfiguration;
}

int main(int argc, char *argv[])
{
#ifdef WIN32
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  po::options_description desc_general("General options");
  command_line::add_arg(desc_general, command_line::arg_help);
  command_line::add_arg(desc_general, command_line::arg_version);

  po::options_description desc_params("Wallet options");
  command_line::add_arg(desc_params, arg_wallet_file);
  command_line::add_arg(desc_params, arg_generate_new_wallet);
  command_line::add_arg(desc_params, arg_password);
  command_line::add_arg(desc_params, arg_daemon_address);
  command_line::add_arg(desc_params, arg_daemon_host);
  command_line::add_arg(desc_params, arg_daemon_port);
  command_line::add_arg(desc_params, arg_command);
  command_line::add_arg(desc_params, arg_log_level);
  command_line::add_arg(desc_params, arg_testnet);
  Tools::wallet_rpc_server::init_options(desc_params);

  po::positional_options_description positional_options;
  positional_options.add(arg_command.name, -1);

  po::options_description desc_all;
  desc_all.add(desc_general).add(desc_params);

  po::variables_map vm;

  bool r = command_line::handle_error_helper(desc_all, [&]() {
    po::store(command_line::parse_command_line(argc, argv, desc_all, true), vm);
    if (command_line::get_arg(vm, command_line::arg_version))
    {
      std::cout << config::get().name << " wallet v" << PROJECT_VERSION_LONG << std::endl;
      return false;
    }

    if (command_line::get_arg(vm, command_line::arg_help))
    {
      std::cout << "Usage: simplewallet [--wallet-file=<file>|--generate-new-wallet=<file>] [--daemon-address=<host>:<port>] [<COMMAND>]";

      std::cout << desc_all << std::endl;
      return false;
    }

    if (command_line::get_arg(vm, arg_testnet))
    {
      config::setType(config::TESTNET);
      return true;
    }
    return true;
  });

  if (!r)
    return 1;
  Logging::LoggerManager logManager;
  Logging::LoggerRef logger(logManager, "simplewallet");
  System::Dispatcher dispatcher;
  //set up logging options
  Level logLevel = DEBUGGING;

  if (command_line::has_arg(vm, arg_log_level))
  {
    logLevel = static_cast<Level>(command_line::get_arg(vm, arg_log_level));
  }

  logManager.configure(buildLoggerConfiguration(logLevel, boost::filesystem::change_extension(argv[0], ".log").string()));

  logger(INFO, BRIGHT_WHITE) << config::get().name << " wallet v" << PROJECT_VERSION_LONG;

  cryptonote::Currency currency = cryptonote::CurrencyBuilder(os::appdata::path(), config::get(), logManager).
                                  currency();

  if (command_line::has_arg(vm, Tools::wallet_rpc_server::arg_rpc_bind_port))
  {
    //runs wallet with rpc interface
    if (!command_line::has_arg(vm, arg_wallet_file))
    {
      logger(ERROR, BRIGHT_RED) << "Wallet file not set.";
      return 1;
    }

    if (!command_line::has_arg(vm, arg_daemon_address))
    {
      logger(ERROR, BRIGHT_RED) << "Daemon address not set.";
      return 1;
    }

    if (!command_line::has_arg(vm, arg_password))
    {
      logger(ERROR, BRIGHT_RED) << "Wallet password not set.";
      return 1;
    }

    std::string wallet_file = command_line::get_arg(vm, arg_wallet_file);
    std::string wallet_password = command_line::get_arg(vm, arg_password);
    std::string daemon_address = command_line::get_arg(vm, arg_daemon_address);
    std::string daemon_host = command_line::get_arg(vm, arg_daemon_host);
    uint16_t daemon_port = command_line::get_arg(vm, arg_daemon_port);
    if (daemon_host.empty())
      daemon_host = "localhost";
    if (!daemon_port)
      daemon_port = config::get().net.rpc_port;

    if (!daemon_address.empty())
    {
      if (!parseUrlAddress(daemon_address, daemon_host, daemon_port))
      {
        logger(ERROR, BRIGHT_RED) << "failed to parse daemon address: " << daemon_address;
        return 1;
      }
    }

    std::unique_ptr<INode> node(new NodeRpcProxy(daemon_host, daemon_port));

    std::promise<std::error_code> errorPromise;
    std::future<std::error_code> error = errorPromise.get_future();
    auto callback = [&errorPromise](std::error_code e) { errorPromise.set_value(e); };
    node->init(callback);
    if (error.get())
    {
      logger(ERROR, BRIGHT_RED) << ("failed to init NodeRPCProxy");
      return 1;
    }

    std::unique_ptr<IWalletLegacy> wallet(new WalletLegacy(currency, *node.get()));

    std::string walletFileName;
    try
    {
      walletFileName = ::tryToOpenWalletOrLoadKeysOrThrow(logger, wallet, wallet_file, wallet_password);

      logger(INFO) << "available balance: " << currency.formatAmount(wallet->actualBalance()) << ", locked amount: " << currency.formatAmount(wallet->pendingBalance());

      logger(INFO, BRIGHT_GREEN) << "Loaded ok";
    }
    catch (const std::exception &e)
    {
      logger(ERROR, BRIGHT_RED) << "Wallet initialize failed: " << e.what();
      return 1;
    }

    Tools::wallet_rpc_server wrpc(dispatcher, logManager, *wallet, *node, walletFileName);

    if (!wrpc.init(vm))
    {
      logger(ERROR, BRIGHT_RED) << "Failed to initialize wallet rpc server";
      return 1;
    }

    Tools::SignalHandler::install([&wrpc] {
      wrpc.send_stop_signal();
    });

    logger(INFO) << "Starting wallet rpc server";
    wrpc.run();
    logger(INFO) << "Stopped wallet rpc server";

    try
    {
      logger(INFO) << "Storing wallet...";
      cryptonote::WalletHelper::storeWallet(*wallet, walletFileName);
      logger(INFO, BRIGHT_GREEN) << "Stored ok";
    }
    catch (const std::exception &e)
    {
      logger(ERROR, BRIGHT_RED) << "Failed to store wallet: " << e.what();
      return 1;
    }
  }
  else
  {
    //runs wallet with console interface
    cryptonote::simple_wallet wal(dispatcher, currency, logManager);

    if (!wal.init(vm))
    {
      logger(ERROR, BRIGHT_RED) << "Failed to initialize wallet";
      return 1;
    }

    std::vector<std::string> command = command_line::get_arg(vm, arg_command);
    if (!command.empty())
      wal.process_command(command);

    Tools::SignalHandler::install([&wal] {
      wal.stop();
    });

    wal.run();

    if (!wal.deinit())
    {
      logger(ERROR, BRIGHT_RED) << "Failed to close wallet";
    }
    else
    {
      logger(INFO) << "Wallet closed";
    }
  }
  return 1;
  //CATCH_ENTRY_L0("main", 1);
}
