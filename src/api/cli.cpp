
#include "cli.h"
#include <Logging/ILogger.h>
#include "arguments.h"

#include "CryptoNoteConfig.h"

using namespace CryptoNote;

using namespace Logging;

namespace api
{

const command_line::arg_descriptor<std::string> arg_address = {"address", "Address", ""};
const command_line::arg_descriptor<std::string> arg_view_key = {"view-key", "View private key", ""};
const command_line::arg_descriptor<std::string> arg_daemon_host = {"daemon-host", "Use daemon instance at host <arg> instead of localhost", "localhost"};
const command_line::arg_descriptor<uint16_t> arg_daemon_port = {"daemon-port", "Use daemon instance at port <arg> instead of 8081",
                                                                RPC_DEFAULT_PORT};
const command_line::arg_descriptor<uint32_t> arg_log_level = {"set_log", "", INFO, true};
const command_line::arg_descriptor<bool> arg_testnet = {"testnet", "Used to deploy test nets. The daemon must be launched with --testnet flag", false}; 

} // namespace api