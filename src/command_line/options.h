#pragma once
#include "common.hpp"

namespace command_line
{

// Software arguments

extern const arg_descriptor<bool> arg_help;
extern const arg_descriptor<bool> arg_version;
extern const arg_descriptor<bool> arg_os_version;

// Core arguments
extern const arg_descriptor<std::string> arg_data_dir;
extern const arg_descriptor<bool> arg_print_genesis_tx;
extern arg_descriptor<std::string> arg_config_file;

// RPC arguments
extern const arg_descriptor<std::string> arg_rpc_bind_ip;
extern arg_descriptor<uint16_t> arg_rpc_bind_port;

// Log/console arguments
extern const arg_descriptor<std::string> arg_log_file;
extern const arg_descriptor<int> arg_log_level;
extern const arg_descriptor<bool> arg_console;

// Test arguments
extern const arg_descriptor<bool> arg_testnet_on;

// Miner arguments
extern const arg_descriptor<std::string> arg_extra_messages;
extern const arg_descriptor<std::string> arg_start_mining;
extern const arg_descriptor<uint32_t> arg_mining_threads;

// P2P arguments
extern const arg_descriptor<std::string> arg_p2p_bind_ip;
extern arg_descriptor<uint16_t> arg_p2p_bind_port;
extern const arg_descriptor<uint16_t> arg_p2p_external_port;
extern const arg_descriptor<bool> arg_p2p_allow_local_ip;
extern const arg_descriptor<std::vector<std::string>> arg_p2p_add_peer;
extern const arg_descriptor<std::vector<std::string>> arg_p2p_add_priority_node;
extern const arg_descriptor<std::vector<std::string>> arg_p2p_add_exclusive_node;
extern const arg_descriptor<std::vector<std::string>> arg_p2p_seed_node;
extern const arg_descriptor<bool> arg_p2p_hide_my_port;

extern const std::string DEFAULT_RPC_IP;
extern const uint16_t DEFAULT_RPC_PORT;

extern void init();

} // namespace command_line