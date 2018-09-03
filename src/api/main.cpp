#include <iostream>

#include <Logging/LoggerManager.h>
#include <CryptoNoteCore/Currency.h>

#include "cli.h"
#include "version.h"

using namespace CryptoNote;

int main(int argc, char *argv[])
{
  Logging::LoggerManager logManager;

  CryptoNote::Currency currency = CryptoNote::CurrencyBuilder(logManager).currency();

  auto helpHandler = [] {
    std::cout << CRYPTONOTE_NAME << " api version " << PROJECT_VERSION_LONG << std::endl;
    std::cout << "Usage: api";
  };

  auto versionHandler = [] {
    std::cout << CRYPTONOTE_NAME << " wallet version " << PROJECT_VERSION_LONG;
  };

  api::Arguments* arg = api::get_argument_handler(argc, argv, helpHandler, versionHandler);

  // arg->init(argc, argv);



  // if (!arg->parseGeneral(helpHandler, versionHandler)) {

  // }

  std::cout << "Inside API main" << std::endl;
}