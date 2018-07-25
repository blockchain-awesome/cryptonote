#include <iostream>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include "Common/CommandLine.h"
#include "Logging/ILogger.h"

#include "WalletRPCService.h"

using namespace std;
using namespace CryptoNote;
using namespace Logging;
namespace po = boost::program_options;

// Description about rpcwalletd

const string VERSION = "1.0.0";

// namespace
// {
// const command_line::arg_descriptor<std::string> arg_daemon_address = {"daemon-address", "Use daemon instance at <host>:<port>", ""};
// const command_line::arg_descriptor<std::string> arg_daemon_host = {"daemon-host", "Use daemon instance at host <arg> instead of localhost", "localhost"};
// const command_line::arg_descriptor<std::string> arg_bind_ip = {"bind-ip", "IP to be bound. Default to localhost", ""};
// const command_line::arg_descriptor<uint16_t> arg_daemon_port = {"daemon-port", "Use daemon instance at port <arg> instead of default one", P2P_DEFAULT_PORT};
// const command_line::arg_descriptor<uint16_t> arg_bind_port = {"bind-port", "Port to be bound", RPC_WALLET_PORT};
// const command_line::arg_descriptor<uint32_t> arg_log_level = {"set_log", "", INFO, true};
// const command_line::arg_descriptor<bool> arg_testnet = {"testnet", "Used to deploy test nets. The daemon must be launched with --testnet flag", false};
// const command_line::arg_descriptor<std::vector<std::string>> arg_command = {"command", ""};

// } // namespace

int main(int argc, char **argv)
{
  // Declare the supported options.
  po::options_description desc_general("General options");
  command_line::add_arg(desc_general, command_line::arg_help);
  command_line::add_arg(desc_general, command_line::arg_version);

  po::options_description desc_params("Wallet options");
  command_line::add_arg(desc_params, arg_bind_ip);
  command_line::add_arg(desc_params, arg_bind_port);
  // command_line::add_arg(desc_params, arg_daemon_address);
  command_line::add_arg(desc_params, arg_daemon_host);
  command_line::add_arg(desc_params, arg_daemon_port);
  command_line::add_arg(desc_params, arg_command);
  command_line::add_arg(desc_params, arg_log_level);
  command_line::add_arg(desc_params, arg_testnet);
  // Tools::wallet_rpc_server::init_options(desc_params);

  po::positional_options_description positional_options;
  positional_options.add(arg_command.name, -1);

  po::options_description desc_all;
  desc_all.add(desc_general).add(desc_params);

  Logging::LoggerManager logManager;
  Logging::LoggerRef logger(logManager, "rpcwalletd");
  System::Dispatcher dispatcher;
  System::Event event;

  po::variables_map vm;

  CryptoNote::Currency currency = CryptoNote::CurrencyBuilder(logManager).currency();
  MultiWalletService::MultiWalletJsonRpcServer multiWallet(dispatcher,
                                                           event, currency, logManager);

  bool r = command_line::handle_error_helper(desc_all, [&]() {
    po::store(command_line::parse_command_line(argc, argv, desc_general, true), vm);

    if (command_line::get_arg(vm, command_line::arg_help))
    {

      std::cout << " rpcwalletd v" << VERSION << std::endl;
      std::cout << "Usage: rpcwalletd [--bind-ip=<ip(127.0.0.1)>] [--bind-port=<port(19888)>] [--daemon-address=<host>:<port>]";
      std::cout << desc_all << '\n'
                << multiWallet.getCommands();
      return false;
    }
    else if (command_line::get_arg(vm, command_line::arg_version))
    {
      std::cout << CRYPTONOTE_NAME << " rpcwalletd v" << VERSION;
      return false;
    }

    auto parser = po::command_line_parser(argc, argv).options(desc_params).positional(positional_options);
    po::store(parser.run(), vm);
    po::notify(vm);
    return true;
  });

  if (!r)
    return 1;

  multiWallet.init(vm);

  // if (!wallet.init(vm))
  // {
  //   logger(ERROR, BRIGHT_RED) << "Failed to initialize wallet";
  //   return 1;
  // }

  // std::vector<std::string> command = command_line::get_arg(vm, arg_command);
  // if (!command.empty())
  //   wal.process_command(command);

  // Tools::SignalHandler::install([&wal] {
  //   wallet.stop();
  // });

  // wallet.run();

  // if (!wallet.deinit())
  // {
  //   logger(ERROR, BRIGHT_RED) << "Failed to close wallet";
  // }
  // else
  // {
  //   logger(INFO) << "Wallet closed";
  // }
  cout << "inside wallet rpc service" << endl;
}