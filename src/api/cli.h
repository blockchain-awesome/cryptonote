

#pragma once

#include "arguments.h"

namespace api
{

extern const command_line::arg_descriptor<std::string> arg_address;
extern const command_line::arg_descriptor<std::string> arg_view_key;
extern const command_line::arg_descriptor<std::string> arg_daemon_host;
extern const command_line::arg_descriptor<uint16_t> arg_daemon_port;
extern const command_line::arg_descriptor<uint32_t> arg_log_level;
extern const command_line::arg_descriptor<bool> arg_testnet;

template <typename F, typename G>
Arguments *get_argument_handler(int argc, char *argv[], F f, G g)
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
  return args;
}
} // namespace api