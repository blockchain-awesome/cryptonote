#pragma once

#include "IWallet.h"

#include <queue>
#include <map>
#include <string>

#include <openssl/sha.h>

#include "wallet/IFusionManager.h"
#include "wallet/WalletIndices.h"

#include <system/Dispatcher.h>
#include <system/Event.h>
#include "transfers/TransfersSynchronizer.h"
#include "transfers/BlockchainSynchronizer.h"

#include "logging/LoggerManager.h"

using namespace cryptonote;

namespace ComplexWallet
{

class WalletManager
{

public:
  WalletManager(System::Dispatcher &dispatcher, const Currency &currency, INode &node, Logging::LoggerManager  &logger);
  virtual ~WalletManager();

  bool createWallet(const account_keys_t &accountKeys);
  void init();

  bool isWalletExisted(const std::string &address);

  bool checkAddress(const std::string &address, account_public_address_t &keys);

  cryptonote::IWalletLegacy *getWallet(const std::string token);

  std::string getAddressesByKeys(const account_public_address_t &keys);

  std::string sha256(const std::string str);

private:
  Logging::LoggerGroup &m_logger;
  System::Dispatcher &m_dispatcher;
  const Currency &m_currency;
  INode &m_node;
  std::map<std::string, void *> m_wallets;
};

} // namespace ComplexWallet
