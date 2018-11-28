// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/os.h"
#include "daemon.h"
#include "logging/ConsoleLogger.h"
#include "common/StringTools.h"
#include "version.h"
#include "cryptonote/core/core.h"
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
  std::string data_dir = get(arg_data_dir);
  std::string config = get(arg_config_file);

  fs::path data_dir_path(data_dir);
  fs::path config_path(config);
  if (!config_path.has_parent_path())
  {
    config_path = data_dir_path.string() + fs::path::preferred_separator + config_path.string();
  }

  boost::system::error_code ec;
  if (fs::exists(config_path, ec))
  {
    po::store(po::parse_config_file<char>(config_path.string<std::string>().c_str(), desc_cmd_sett), vm);
  }
}

fs::path Daemon::getLogFile()
{
  auto cfgLogFile = fs::path(get(arg_log_file));

  if (cfgLogFile.empty())
  {
    cfgLogFile = fs::change_extension(exeFile, ".log").string();
  }
  else
  {
    if (!cfgLogFile.parent_path().empty())
    {
      cfgLogFile = exeFile.root_path().string() + cfgLogFile.string();
    }
  }
  return cfgLogFile;
}

/**
 *  Generate A Genesis TX 
 */
void Daemon::printGenesisTx()
{
  Logging::ConsoleLogger logger;
  cryptonote::transaction_t tx = cryptonote::CurrencyBuilder(os::appdata::path(), config::get(), logger).generateGenesisTransaction();
  binary_array_t txb = cryptonote::BinaryArray::to(tx);
  std::string tx_hex = hex::toString(txb);

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
    std::cout << config::get().name << " v" << PROJECT_VERSION_LONG << ENDL;
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
    std::cout << config::get().name << " v" << PROJECT_VERSION_LONG << ENDL << ENDL;
    std::cout << desc_options << std::endl;
    return false;
  }
  return true;
}

} // namespace command_line
