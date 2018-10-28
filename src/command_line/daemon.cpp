// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/os.h"
#include "command_line/common.h"
#include "daemon.h"
#include "logging/ConsoleLogger.h"
#include "common/StringTools.h"
#include "version.h"
#include "cryptonote/core/Core.h"
#include "cryptonote/core/CryptoNoteTools.h"

namespace command_line
{

const arg_descriptor<std::string> arg_config_file = {"config-file", "Specify configuration file", std::string(cryptonote::CRYPTONOTE_NAME) + ".conf"};
const arg_descriptor<bool> arg_os_version = {"os-version", ""};
const arg_descriptor<std::string> arg_log_file = {"log-file", "", ""};
const arg_descriptor<int> arg_log_level = {"log-level", "", 2}; // info level
const arg_descriptor<bool> arg_console = {"no-console", "Disable daemon console commands"};
const arg_descriptor<bool> arg_testnet_on = {"testnet", "Used to deploy test nets. Checkpoints and hardcoded seeds are ignored, "
                                                        "network id is changed. Use it with --data-dir flag. The wallet must be launched with --testnet flag.",
                                             false};
const arg_descriptor<bool> arg_print_genesis_tx = {"print-genesis-tx", "Prints genesis' block tx hex to insert it to config and exits"};

Daemon::Daemon() : desc_options("Allowed options"),
                   desc_cmd_only("Command line desc_cmd_sett"),
                   desc_cmd_sett("Command line desc_cmd_sett and settings desc_cmd_sett")
{
    configFolder = os::appdata::path();

    add_arg(desc_cmd_only, arg_help);
    add_arg(desc_cmd_only, arg_version);
    add_arg(desc_cmd_only, arg_os_version);
    // tools::get_default_data_dir() can't be called during static initialization
    add_arg(desc_cmd_only, arg_data_dir, os::appdata::path());
    add_arg(desc_cmd_only, arg_config_file);

    add_arg(desc_cmd_sett, arg_log_file);
    add_arg(desc_cmd_sett, arg_log_level);
    add_arg(desc_cmd_sett, arg_console);
    add_arg(desc_cmd_sett, arg_testnet_on);
    add_arg(desc_cmd_sett, arg_print_genesis_tx);
    desc_options.add(desc_cmd_only).add(desc_cmd_sett);
}

std::string Daemon::getGenesisName()
{
    return arg_print_genesis_tx.name;
}

void Daemon::prinGenesisTxHex()
{
    Logging::ConsoleLogger logger;
    cryptonote::Transaction tx = cryptonote::CurrencyBuilder(logger, os::appdata::path()).generateGenesisTransaction();
    cryptonote::BinaryArray txb = cryptonote::toBinaryArray(tx);
    std::string tx_hex = Common::toHex(txb);

    std::cout << "Insert this line into your coin configuration file as is: " << std::endl;
    std::cout << "const char GENESIS_COINBASE_TX_HEX[] = \"" << tx_hex << "\";" << std::endl;
}

void Daemon::help()
{
    std::cout << cryptonote::CRYPTONOTE_NAME << " v" << PROJECT_VERSION_LONG << ENDL << ENDL;
    std::cout << desc_options << std::endl;
}

void Daemon::assign()
{
    if (vm.count(arg_data_dir.name) != 0 &&
        (!vm[arg_data_dir.name].defaulted()))
    {
        configFolder = get_arg(vm, arg_data_dir);
        configFolderDefaulted = vm[arg_data_dir.name].defaulted();
    }
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

std::string Daemon::getLogFile()
{
    return get_arg(vm, arg_log_file);
}

bool Daemon::getTestMode()
{
    return get_arg(vm, arg_testnet_on);
}

bool Daemon::getConsole()
{
    return get_arg(vm, arg_console);
}

int Daemon::getLogLevel()
{
    return get_arg(vm, arg_log_level);
}

} // namespace command_line
