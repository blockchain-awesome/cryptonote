// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/os.h"
#include "daemon.h"
#include "logging/ConsoleLogger.h"
#include "common/StringTools.h"
#include "version.h"
#include "cryptonote/core/Core.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "CryptoNoteConfig.h"

namespace command_line
{
bool Daemon::innerParse()
{
  if (!parseHelp())
  {
    return false;
  }
  if (get(arg_print_genesis_tx))
  {
    printGenesisTx();
    return false;
  }
  parseConfigFile();
  return true;
}

void Daemon::parseConfigFile()
{
  // std::string data_dir = get_arg(vm, arg_data_dir);
  // std::string config = get_arg(vm, arg_config_file);

  // boost::filesystem::path data_dir_path(data_dir);
  // boost::filesystem::path config_path(config);
  // if (!config_path.has_parent_path())
  // {
  //   config_path = data_dir_path / config_path;
  // }

  // boost::system::error_code ec;
  // if (boost::filesystem::exists(config_path, ec))
  // {
  //   po::store(po::parse_config_file<char>(config_path.string<std::string>().c_str(), desc_cmd_sett), vm);
  // }
}

void Daemon::printGenesisTx()
{
  Logging::ConsoleLogger logger;
  cryptonote::Transaction tx = cryptonote::CurrencyBuilder(logger, os::appdata::path()).generateGenesisTransaction();
  cryptonote::BinaryArray txb = cryptonote::toBinaryArray(tx);
  std::string tx_hex = Common::toHex(txb);

  std::cout << "Insert this line into your coin configuration file as is: " << std::endl;
  std::cout << "const char GENESIS_COINBASE_TX_HEX[] = \"" << tx_hex << "\";" << std::endl;

  return;
}

void Daemon::init()
{
  addCommand(arg_help);
  addCommand(arg_version);
  addCommand(arg_os_version);
  addCommand(arg_data_dir, os::appdata::path());
  addCommand(arg_config_file);

  addSetting(arg_log_file);
  addSetting(arg_log_level);
  addSetting(arg_console);
  addSetting(arg_testnet_on);
  addSetting(arg_print_genesis_tx);
}

bool Daemon::checkVersion()
{
  bool exit = false;

  if (get_arg(vm, arg_version))
  {
    std::cout << cryptonote::CRYPTONOTE_NAME << " v" << PROJECT_VERSION_LONG << ENDL;
    exit = true;
  }
  if (get_arg(vm, arg_os_version))
  {
    std::cout << "OS: " << os::version::get() << ENDL;
    exit = true;
  }

  if (exit)
  {
    return true;
  }

  return false;
}

bool Daemon::parseHelp()
{
  if (get_arg(vm, arg_help))
  {
    std::cout << cryptonote::CRYPTONOTE_NAME << " v" << PROJECT_VERSION_LONG << ENDL << ENDL;
    std::cout << desc_options << std::endl;
    return false;
  }
  return true;
}

} // namespace command_line
