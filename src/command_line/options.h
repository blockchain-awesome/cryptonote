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
extern const arg_descriptor<std::string> arg_config_file;

// RPC arguments
extern const arg_descriptor<std::string> arg_rpc_bind_ip;
extern const arg_descriptor<uint16_t> arg_rpc_bind_port;

// Log/console arguments
extern const arg_descriptor<std::string> arg_log_file;
extern const arg_descriptor<int> arg_log_level; 
extern const arg_descriptor<bool> arg_console;

// Test arguments
extern const arg_descriptor<bool> arg_testnet_on;

// Miner arguments
extern const arg_descriptor<std::string> arg_extra_messages;
extern const arg_descriptor<std::string> arg_start_mining;
extern const arg_descriptor<uint32_t>    arg_mining_threads;

extern const std::string DEFAULT_RPC_IP;
extern const uint16_t DEFAULT_RPC_PORT;

          
} // namespace command_line