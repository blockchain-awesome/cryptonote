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
    return true;
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

bool Daemon::checkVersion() {
  bool exit = false;

  if (get_arg(vm, arg_version)) {
    std::cout << cryptonote::CRYPTONOTE_NAME << " v" << PROJECT_VERSION_LONG << ENDL;
    exit = true;
  }
  if (get_arg(vm, arg_os_version)) {
    std::cout << "OS: " << os::version::get() << ENDL;
    exit = true;
  }

  if (exit) {
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
