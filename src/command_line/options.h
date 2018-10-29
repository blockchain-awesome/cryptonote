#pragma once
#include "common.hpp"

namespace command_line
{

extern const arg_descriptor<bool> arg_help;
extern const arg_descriptor<bool> arg_version;
extern const arg_descriptor<std::string> arg_data_dir;
extern const arg_descriptor<bool> arg_print_genesis_tx;
extern const arg_descriptor<std::string> arg_config_file;
extern const arg_descriptor<std::string> arg_rpc_bind_ip;
extern const arg_descriptor<uint16_t> arg_rpc_bind_port;
extern const arg_descriptor<bool> arg_os_version;
extern const arg_descriptor<std::string> arg_log_file;
extern const arg_descriptor<int> arg_log_level; 
extern const arg_descriptor<bool> arg_console;
extern const arg_descriptor<bool> arg_testnet_on;

extern const std::string DEFAULT_RPC_IP;
extern const uint16_t DEFAULT_RPC_PORT;

          
} // namespace command_line