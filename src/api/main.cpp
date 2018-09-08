#include <iostream>

#include <Logging/LoggerManager.h>
#include <CryptoNoteCore/Currency.h>

#include "cli.h"
#include "version.h"
#include "node.h"
#include "keys.h"

int main(int argc, char *argv[])
{
  Logging::LoggerManager logManager;
  CryptoNote::Currency currency = CryptoNote::CurrencyBuilder(logManager).currency();
  std::unique_ptr<api::Node> node;
  std::unique_ptr<api::Account> account;

  auto helpHandler = [] {
    std::cout << CryptoNote::CRYPTONOTE_NAME << " api version " << PROJECT_VERSION_LONG << std::endl;
    std::cout << "Usage: api" << std::endl;
  };

  auto versionHandler = [] {
    std::cout << CryptoNote::CRYPTONOTE_NAME << " api version " << PROJECT_VERSION_LONG << std::endl;
  };

  auto parameterHandler = [&](po::variables_map &vm) {
    std::cout << "inside parameter handling" << PROJECT_VERSION_LONG << std::endl;
    api::ParsedParameters p(vm);
    std::cout << " is parsed: " << p.preparedAccount() << std::endl;
    node = std::unique_ptr<api::Node>(new api::Node(p.daemon_host, p.daemon_port));

    if (!p.daemon_host.empty() && p.daemon_port)
    {
      std::cout << " starting node. " << std::endl;
      if (!node->init(currency))
      {
        std::cout << "failed to init NodeRPCProxy" << std::endl;
      }
      std::cout << "started node" << std::endl;
    }

    if (p.preparedAccount())
    {
      std::cout << "inside account prepared!" << std::endl;
      account = std::unique_ptr<api::Account>(new api::Account(p.spend_key, p.view_key));
      node->initAccount(account->toKeys());
    }
    return true;
  };

  api::Arguments *arg = api::get_argument_handler(argc, argv, helpHandler, versionHandler, parameterHandler);

  size_t count = node->getPeerCount();

  std::cout << "peer:" << count << std::endl;

  node->wait(1000 * 10);

  count = node->getPeerCount();

  std::cout << "peer:" << count << std::endl;

  std::cout << "Node Successfully Connected!" << std::endl;

  std::cout << "Inside API main" << std::endl;
}