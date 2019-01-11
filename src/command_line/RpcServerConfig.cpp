// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "RpcServerConfig.h"
#include "config/common.h"
#include "command_line/options.h"
#include "CryptoNoteConfig.h"

namespace cryptonote {
  RpcServerConfig::RpcServerConfig() : bindIp("0.0.0.0"), bindPort(config::get().net.rpc_port) {
  }

  std::string RpcServerConfig::getBindAddress() const {
    return bindIp + ":" + std::to_string(bindPort);
  }
  
  void RpcServerConfig::initOptions(boost::program_options::options_description& desc) {
	command_line::init();
    command_line::add_arg(desc, command_line::arg_rpc_bind_ip);
    command_line::add_arg(desc, command_line::arg_rpc_bind_port);
    command_line::add_arg(desc, command_line::arg_enable_cors);
  }

  void RpcServerConfig::init(const boost::program_options::variables_map& vm)  {
    bindIp = command_line::get_arg(vm, command_line::arg_rpc_bind_ip);
    bindPort = command_line::get_arg(vm, command_line::arg_rpc_bind_port);
    enableCORS = command_line::get_arg(vm, command_line::arg_enable_cors);
    bool defaulted = vm[command_line::arg_data_dir.name].defaulted();
    if (defaulted) {
      bindPort = config::get().net.rpc_port;
    }
  }

}
