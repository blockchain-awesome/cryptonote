

#pragma once

#include "arguments.h"

namespace api
{

extern const command_line::arg_descriptor<std::string> arg_address;
extern const command_line::arg_descriptor<std::string> arg_spend_key;
extern const command_line::arg_descriptor<std::string> arg_view_key;
extern const command_line::arg_descriptor<std::string> arg_daemon_host;
extern const command_line::arg_descriptor<uint16_t> arg_daemon_port;
extern const command_line::arg_descriptor<uint32_t> arg_log_level;
extern const command_line::arg_descriptor<bool> arg_testnet;

class ParsedParameters
{
public:

  ParsedParameters(po::variables_map &vm)
  {
    try
    {
      address = command_line::get_arg(vm, arg_address);
      view_key = command_line::get_arg(vm, arg_view_key);
      spend_key = command_line::get_arg(vm, arg_spend_key);
      daemon_host = command_line::get_arg(vm, arg_daemon_host);
      daemon_port = command_line::get_arg(vm, arg_daemon_port);
    }
    catch (std::exception &e)
    {
      std::cout << "parameter error: " << e.what() << std::endl;
    }
  }

  bool preparedAccount() {
    return (!address.empty() && !view_key.empty() && !spend_key.empty());
  }

  std::string address;
  std::string view_key;
  std::string spend_key;
  std::string daemon_host;
  uint16_t daemon_port;
  uint32_t log_level;
  bool testnet;
};

template <typename F, typename G, typename H>
Arguments *get_argument_handler(int argc, char *argv[], F f, G g, H h)
{
  Options *general_ptr = new Options("General options");

  Options general = *general_ptr;
  general.add(command_line::arg_help);
  general.add(command_line::arg_version);

  // std::cout << *general.getDesc() << std::endl;

  Options *parameter_ptr = new Options("API options");
  Options parameter = *parameter_ptr;

  // api::Options parameter("API options");

  parameter.add(api::arg_address);
  parameter.add(api::arg_spend_key);
  parameter.add(api::arg_view_key);
  parameter.add(api::arg_daemon_host);
  parameter.add(api::arg_daemon_port);
  parameter.add(api::arg_log_level);
  parameter.add(api::arg_testnet);

  // std::cout << *parameter.getDesc() << std::endl;

  Arguments *args = new Arguments(general_ptr, parameter_ptr);
  args->init(argc, argv);
  if (!args->parseGeneral(f, g))
  {
    return nullptr;
  }

  if (!args->parseParameter(h))
  {
    return nullptr;
  }
  return args;
}
} // namespace api