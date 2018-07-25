
#pragma once

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include "Common/CommandLine.h"
#include "Logging/ILogger.h"
#include "CryptoNoteConfig.h"

using namespace CryptoNote;
using namespace Logging;

namespace
{
// const command_line::arg_descriptor<std::string> arg_daemon_address = {"daemon-address", "Use daemon instance at <host>:<port>", ""};
const command_line::arg_descriptor<std::string> arg_daemon_host = {"daemon-host", "Use daemon instance at host <arg> instead of localhost", "localhost"};
const command_line::arg_descriptor<std::string> arg_bind_ip = {"ip", "IP to be bound. Default to localhost", "127.0.0.1"};
const command_line::arg_descriptor<uint16_t> arg_daemon_port = {"daemon-port", "Use daemon instance at port <arg> instead of default one", P2P_DEFAULT_PORT};
const command_line::arg_descriptor<uint16_t> arg_bind_port = {"port", "Port to be bound", RPC_WALLET_PORT};
const command_line::arg_descriptor<uint32_t> arg_log_level = {"set_log", "", INFO, true};
const command_line::arg_descriptor<bool> arg_testnet = {"testnet", "Used to deploy test nets. The daemon must be launched with --testnet flag", false};
const command_line::arg_descriptor<std::vector<std::string>> arg_command = {"command", ""};

} // namespace