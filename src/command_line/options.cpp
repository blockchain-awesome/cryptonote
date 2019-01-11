// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CryptoNoteConfig.h"
#include "options.h"
#include "config/common.h"

namespace command_line
{
const arg_descriptor<bool> arg_help = {"help", "Produce help message"};
const arg_descriptor<bool> arg_version = {"version", "Output version information"};
const arg_descriptor<bool> arg_os_version = {"os-version", ""};

// Storage
const arg_descriptor<std::string> arg_data_dir = {"data-dir", "Specify data directory"};
arg_descriptor<std::string> arg_config_file;

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

// RPC
const arg_descriptor<std::string> arg_rpc_bind_ip = {"rpc-bind-ip", "", DEFAULT_RPC_IP};
arg_descriptor<uint16_t> arg_rpc_bind_port;
const arg_descriptor<std::vector<std::string>> arg_enable_cors = { "enable-cors", "Adds header 'Access-Control-Allow-Origin' to the daemon's RPC responses. Uses the value as domain. Use * for all" };

// P2P
const arg_descriptor<std::string> arg_p2p_bind_ip        = {"p2p-bind-ip", "Interface for p2p network protocol", "0.0.0.0"};
arg_descriptor<uint16_t>    arg_p2p_bind_port;
const arg_descriptor<uint16_t>    arg_p2p_external_port = { "p2p-external-port", "External port for p2p network protocol (if port forwarding used with NAT)", 0 };
const arg_descriptor<bool>        arg_p2p_allow_local_ip = {"allow-local-ip", "Allow local ip add to peer list, mostly in debug purposes"};
const arg_descriptor<std::vector<std::string> > arg_p2p_add_peer   = {"add-peer", "Manually add peer to local peerlist"};
const arg_descriptor<std::vector<std::string> > arg_p2p_add_priority_node   = {"add-priority-node", "Specify list of peers to connect to and attempt to keep the connection open"};
const arg_descriptor<std::vector<std::string> > arg_p2p_add_exclusive_node   = {"add-exclusive-node", "Specify list of peers to connect to only."
      " If this option is given the options add-priority-node and seed-node are ignored"};
const arg_descriptor<std::vector<std::string> > arg_p2p_seed_node   = {"seed-node", "Connect to a node to retrieve peer addresses, and disconnect"};
const arg_descriptor<bool> arg_p2p_hide_my_port   =    {"hide-my-port", "Do not announce yourself as peerlist candidate", false, true};

bool initialized = false;

void init() {
	if (initialized) {
		return;
	}
    arg_rpc_bind_port = {"rpc-bind-port", "", config::get().net.rpc_port};
    arg_config_file = {"config-file", "Specify configuration file", std::string(config::get().name) + ".conf"};
    arg_p2p_bind_port      = {"p2p-bind-port", "Port for p2p network protocol", config::get().net.p2p_port};
	initialized = true;
}

} // namespace command_line
