#include <iostream>

#include <Logging/LoggerManager.h>
#include <CryptoNoteCore/Currency.h>

#include "cli.h"
#include "version.h"
#include "node.h"

int main(int argc, char *argv[])
{
  Logging::LoggerManager logManager;

  CryptoNote::Currency currency = CryptoNote::CurrencyBuilder(logManager).currency();

  std::unique_ptr<api::Node> node;

  auto helpHandler = [] {
    std::cout << CryptoNote::CRYPTONOTE_NAME << " api version " << PROJECT_VERSION_LONG << std::endl;
    std::cout << "Usage: api" << std::endl;
  };

  auto versionHandler = [] {
    std::cout << CryptoNote::CRYPTONOTE_NAME << " api version " << PROJECT_VERSION_LONG << std::endl;
  };

  auto parameterHandler = [&node](po::variables_map &vm) {
    std::cout << "inside parameter handling" << PROJECT_VERSION_LONG << std::endl;
    api::ParsedParameters p(vm);
    std::cout << " is parsed: " << p.prepared() << std::endl;

    if (!p.daemon_host.empty() && p.daemon_port)
    {
      node = std::unique_ptr<api::Node>(new api::Node(p.daemon_host, p.daemon_port));
    }
    return true;
  };

  api::Arguments *arg = api::get_argument_handler(argc, argv, helpHandler, versionHandler, parameterHandler);

  if (!node->init())
  {
    std::cout << "failed to init NodeRPCProxy" << std::endl;
  }

  std::cout << "Node Successfully Connected!" << std::endl;

  std::cout << "Inside API main" << std::endl;
}