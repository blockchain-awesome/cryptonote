// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CryptoNoteConfig.h"
#include "options.h"

namespace command_line
{
const arg_descriptor<bool> arg_help = {"help", "Produce help message"};
const arg_descriptor<bool> arg_version = {"version", "Output version information"};
const arg_descriptor<bool> arg_os_version = {"os-version", ""};

// Storage
const arg_descriptor<std::string> arg_data_dir = {"data-dir", "Specify data directory"};
const arg_descriptor<std::string> arg_config_file = {"config-file", "Specify configuration file", std::string(cryptonote::CRYPTONOTE_NAME) + ".conf"};

// Log info
const arg_descriptor<std::string> arg_log_file = {"log-file", "", ""};
const arg_descriptor<int> arg_log_level = {"log-level", "", 2}; // info level

// Console
const arg_descriptor<bool> arg_console = {"no-console", "Disable daemon console commands"};

// Test net
const arg_descriptor<bool> arg_testnet_on = {"testnet", "Used to deploy test nets. Checkpoints and hardcoded seeds are ignored, "
                                                        "network id is changed. Use it with --data-dir flag. The wallet must be launched with --testnet flag.",
                                             false};
// BLockchain
const arg_descriptor<bool> arg_print_genesis_tx = {"print-genesis-tx", "Prints genesis' block tx hex to insert it to config and exits"};
const arg_descriptor<std::string> arg_extra_messages =  {"extra-messages-file", "Specify file for extra messages to include into coinbase transactions", "", true};

// Mining
const arg_descriptor<std::string> arg_start_mining =    {"start-mining", "Specify wallet address to mining for", "", true};
const arg_descriptor<uint32_t>    arg_mining_threads =  {"mining-threads", "Specify mining threads count", 0, true};


const std::string DEFAULT_RPC_IP = "127.0.0.1";
const uint16_t DEFAULT_RPC_PORT = cryptonote::RPC_DEFAULT_PORT;

// RPC
const arg_descriptor<std::string> arg_rpc_bind_ip = {"rpc-bind-ip", "", DEFAULT_RPC_IP};
const arg_descriptor<uint16_t> arg_rpc_bind_port = {"rpc-bind-port", "", DEFAULT_RPC_PORT};

} // namespace command_line
