#include "WalletManager.h"
#include <algorithm>
#include <ctime>
#include <cassert>
#include <numeric>
#include <random>
#include <set>
#include <tuple>
#include <utility>

#include <iostream>

#include <System/EventLock.h>
#include <System/RemoteContext.h>

#include "Logging/LoggerGroup.h"

#include "ITransaction.h"

#include "common/ScopeExit.h"
#include "common/ShuffleGenerator.h"
#include "common/StdInputStream.h"
#include "common/StdOutputStream.h"
#include "common/StringTools.h"
#include "cryptonote/core/Account.h"
#include "cryptonote/core/Currency.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/TransactionApi.h"
#include "crypto/crypto.h"
#include "Transfers/TransfersContainer.h"
#include "Wallet/WalletSerialization.h"
#include "Wallet/WalletErrors.h"
#include "Wallet/WalletUtils.h"
#include "WalletLegacy/WalletHelper.h"

#include "SingleWallet.h"
#include "WalletLegacy/WalletLegacy.h"

using namespace Common;
using namespace crypto;
using namespace cryptonote;

using namespace std;

using namespace cryptonote;

namespace ComplexWallet
{

WalletManager::WalletManager(System::Dispatcher &dispatcher, const Currency &currency, INode &node, Logging::LoggerManager &logger) : m_dispatcher(dispatcher),
                                                                                                                                      m_currency(currency),
                                                                                                                                      m_node(node),
                                                                                                                                      m_logger(logger)
{
}

WalletManager::~WalletManager()
{
}

bool WalletManager::createWallet(const AccountKeys &accountKeys)
{

  Logging::LoggerRef(m_logger, "inteface")(Logging::INFO) << "creating new wallet" << endl;
  cryptonote::IWalletLegacy *wallet = new SingleWallet(m_currency, m_node);
  // cryptonote::IWalletLegacy *wallet = new WalletLegacy(m_currency, m_node);

  // WalletHelper::InitWalletResultObserver initObserver;
  // std::future<std::error_code> f_initError = initObserver.initResult.get_future();

  // WalletHelper::IWalletRemoveObserverGuard removeGuard(*wallet, initObserver);

  std::string address = getAddressesByKeys(accountKeys.address);
  m_wallets[address] = wallet;

  // wallet->initWithKeys(accountKeys, "");

  return true;
}

bool WalletManager::isWalletExisted(const std::string &address)
{
  void *wallet = m_wallets[address];
  if (wallet)
  {
    return true;
  }
  return false;
}

bool WalletManager::checkAddress(const std::string &address, AccountPublicAddress &keys)
{

  std::string genAddress = m_currency.accountAddressAsString(keys);

  if (genAddress != address)
  {
    return false;
  }
  return true;
}

cryptonote::IWalletLegacy *WalletManager::getWallet(const std::string address)
{
  std::map<std::string, void *>::iterator it;
  it = m_wallets.find(address);
  if (it != m_wallets.end())
  {
    return (cryptonote::IWalletLegacy *)it->second;
  }
  return NULL;
}

std::string WalletManager::getAddressesByKeys(const AccountPublicAddress &keys)
{
  return m_currency.accountAddressAsString(keys);
}

std::string WalletManager::sha256(const std::string str)
{
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, str.c_str(), str.size());
  SHA256_Final(hash, &sha256);
  int i = 0;
  char outputBuffer[65];
  for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
    sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
  }
  outputBuffer[64] = 0;
  std::string res = outputBuffer;
  return res;
}

} // namespace ComplexWallet
