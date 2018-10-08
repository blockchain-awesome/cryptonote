#include <ctime>
#include <fstream>
#include <future>
#include <iomanip>
#include <thread>
#include <set>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "common/CommandLine.h"
#include "common/SignalHandler.h"
#include "common/StringTools.h"
#include "common/PathTools.h"
#include "common/Util.h"
#include "CryptoNoteCore/CryptoNoteFormatUtils.h"
#include "CryptoNoteProtocol/CryptoNoteProtocolHandler.h"
#include "NodeRpcProxy/NodeRpcProxy.h"
#include "Rpc/CoreRpcServerCommandsDefinitions.h"
#include "Rpc/HttpClient.h"

#include "Wallet/WalletRpcServer.h"
#include "WalletLegacy/WalletLegacy.h"
#include "Wallet/LegacyKeysImporter.h"
#include "WalletLegacy/WalletHelper.h"

#include "version.h"

#include <Logging/LoggerManager.h>

#include "args.h"
#include "ComplexWallet.h"
#include "WalletManager.h"
#include "WalletFile.h"

using namespace ComplexWallet;

namespace po = boost::program_options;

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
  command_line::add_arg(desc_params, arg_generate_wallet);
  command_line::add_arg(desc_params, arg_address);
  command_line::add_arg(desc_params, arg_send_key);
  command_line::add_arg(desc_params, arg_view_key);

  command_line::add_arg(desc_params, arg_password);
  command_line::add_arg(desc_params, arg_daemon_address);
  command_line::add_arg(desc_params, arg_daemon_host);
  command_line::add_arg(desc_params, arg_daemon_port);
  command_line::add_arg(desc_params, arg_bind_host);
  command_line::add_arg(desc_params, arg_bind_port);
  command_line::add_arg(desc_params, arg_command);
  command_line::add_arg(desc_params, arg_log_level);
  command_line::add_arg(desc_params, arg_testnet);
  Tools::wallet_rpc_server::init_options(desc_params);

  po::positional_options_description positional_options;
  positional_options.add(arg_command.name, -1);

  po::options_description desc_all;
  desc_all.add(desc_general).add(desc_params);

  Logging::LoggerManager logManager;
  Logging::LoggerRef logger(logManager, "complexwallet");
  System::Dispatcher dispatcher;

  po::variables_map vm;

  bool r = command_line::handle_error_helper(desc_all, [&]() {
    po::store(command_line::parse_command_line(argc, argv, desc_general, true), vm);

    if (command_line::get_arg(vm, command_line::arg_help))
    {
      CryptoNote::Currency tmp_currency = CryptoNote::CurrencyBuilder(logManager).currency();
      CryptoNote::complex_wallet tmp_wallet(dispatcher, tmp_currency, logManager);

      std::cout << CRYPTONOTE_NAME << " wallet version " << PROJECT_VERSION_LONG << std::endl;
      std::cout << "Usage: complexwallet [--wallet-file=<file>|--generate-new-wallet=<file>] [--daemon-address=<host>:<port>] [<COMMAND>]";
      std::cout << desc_all << '\n'
                << tmp_wallet.get_commands_str();
      return false;
    }
    else if (command_line::get_arg(vm, command_line::arg_version))
    {
      std::cout << CRYPTONOTE_NAME << " wallet version " << PROJECT_VERSION_LONG;
      return false;
    }

    auto parser = po::command_line_parser(argc, argv).options(desc_params).positional(positional_options);
    po::store(parser.run(), vm);
    po::notify(vm);
    return true;
  });

  if (!r)
    return 1;

  //set up logging options
  Level logLevel = DEBUGGING;

  if (command_line::has_arg(vm, arg_log_level))
  {
    logLevel = static_cast<Level>(command_line::get_arg(vm, arg_log_level));
  }

  logManager.configure(buildLoggerConfiguration(logLevel, Common::ReplaceExtenstion(argv[0], ".log")));

  logger(INFO, BRIGHT_WHITE) << CRYPTONOTE_NAME << " wallet version " << PROJECT_VERSION_LONG;

  CryptoNote::Currency currency = CryptoNote::CurrencyBuilder(logManager).testnet(command_line::get_arg(vm, arg_testnet)).currency();

  //runs wallet with console interface
  CryptoNote::complex_wallet wal(dispatcher, currency, logManager);

  std::string genWallet = command_line::get_arg(vm, arg_generate_wallet);
  std::string sendKey = command_line::get_arg(vm, arg_send_key);
  std::string viewKey = command_line::get_arg(vm, arg_view_key);

  if (genWallet.length() && sendKey.length() && viewKey.length())
  {
    std::string address = command_line::get_arg(vm, arg_address);
    std::string password = command_line::get_arg(vm, arg_password);
    create_wallet_by_keys(genWallet, password, address, sendKey, viewKey, logger);
    return 0;
  }

  System::Event localStopEvent(dispatcher);

  WalletManager *wm = new WalletManager(dispatcher, currency, *wal.get_node(), logManager);

  ComplexWalletServer *cws = new ComplexWalletServer(dispatcher, localStopEvent, logManager, *wm);

  std::string bind_host = command_line::get_arg(vm, arg_bind_host);
  uint16_t bind_port = command_line::get_arg(vm, arg_bind_port);

  std::cout << "bind host: " << bind_host << ", bind port: " << bind_port << std::endl;

  LoggerRef log = logger;

  log(Logging::INFO) << "starting rpc server";
  cws->start(bind_host, bind_port);
  log(Logging::INFO) << "started rpc server";

  std::cout << "before init" << std::endl;

  std::promise<std::error_code> errorPromise;
  std::future<std::error_code> f_error = errorPromise.get_future();
  auto callback = [&errorPromise](std::error_code e) { errorPromise.set_value(e); };

  cws->init(callback);
  auto error = f_error.get();
  if (error)
  {
    log(Logging::INFO) << "failed to init Rpc Server! " << error.message();
    return false;
  }

  log(Logging::INFO) << "successfully init rpc server!";

  if (!wal.init(vm))
  {
    logger(ERROR, BRIGHT_RED) << "Failed to initialize wallet";
    return 1;
  }

  std::cout << "after init" << std::endl;
  std::vector<std::string> command = command_line::get_arg(vm, arg_command);
  if (!command.empty())
    wal.process_command(command);

  Tools::SignalHandler::install([&wal, &cws, &log] {
    wal.stop();
    cws->stop();
    log(Logging::INFO) << "Complex server stopped!";
  });

  logger(INFO) << "before runing.";

  wal.run();

  logger(INFO) << "after running.";

  if (!wal.deinit())
  {
    logger(ERROR, BRIGHT_RED) << "Failed to close wallet";
  }
  else
  {
    logger(INFO) << "Wallet closed";
  }

  cws->stop();
  log(Logging::INFO) << "Complex server is shutdown!";

  return 1;
  //CATCH_ENTRY_L0("main", 1);
}