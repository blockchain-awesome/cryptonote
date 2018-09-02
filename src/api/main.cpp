#include <iostream>

#include <Logging/LoggerManager.h>
#include <CryptoNoteCore/Currency.h>

#include "arguments.h"

using namespace api;

int main(int argc, char *argv[])
{
  Logging::LoggerManager logManager;

  CryptoNote::Currency currency = CryptoNote::CurrencyBuilder(logManager).currency();

  api::Options general("General options");

  general.add(command_line::arg_help);
  general.add(command_line::arg_version);

  api::Options parameter("API options");
  parameter.add(api::arg_address);
  parameter.add(api::arg_view_key);
  parameter.add(api::arg_daemon_host);
  parameter.add(api::arg_daemon_port);
  parameter.add(api::arg_log_level);
  parameter.add(api::arg_testnet);

  api::Arguments Arguments(general, parameter);

  std::cout << "Inside API main" << std::endl;
}