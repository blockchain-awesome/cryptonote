#pragma once

#include "IWallet.h"

#include <queue>
#include <map>
#include <string>

#include <openssl/sha.h>

#include "Wallet/IFusionManager.h"
#include "Wallet/WalletIndices.h"

#include <System/Dispatcher.h>
#include <System/Event.h>
#include "Transfers/TransfersSynchronizer.h"
#include "Transfers/BlockchainSynchronizer.h"

#include "Logging/LoggerManager.h"

using namespace cryptonote;

namespace ComplexWallet
{

class WalletManager
{

public:
  WalletManager(System::Dispatcher &dispatcher, const Currency &currency, INode &node, Logging::LoggerManager  &logger);
  virtual ~WalletManager();

  bool createWallet(const AccountKeys &accountKeys);
  void init();

  bool isWalletExisted(const std::string &address);

  bool checkAddress(const std::string &address, AccountPublicAddress &keys);

  cryptonote::IWalletLegacy *getWallet(const std::string token);

  std::string getAddressesByKeys(const AccountPublicAddress &keys);

  std::string sha256(const std::string str);

private:
  Logging::LoggerGroup &m_logger;
  System::Dispatcher &m_dispatcher;
  const Currency &m_currency;
  INode &m_node;
  std::map<std::string, void *> m_wallets;
};

} // namespace ComplexWallet
