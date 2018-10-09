// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
#include "cryptonote_core/Account.h"
#include "cryptonote_core/Currency.h"
#include "cryptonote_core/CryptoNoteFormatUtils.h"
#include "cryptonote_core/CryptoNoteTools.h"
#include "cryptonote_core/TransactionApi.h"
#include "crypto/crypto.h"
#include "Transfers/TransfersContainer.h"
#include "Wallet/WalletSerialization.h"
#include "Wallet/WalletErrors.h"
#include "Wallet/WalletUtils.h"
#include "WalletLegacy/WalletHelper.h"


#include "WalletInterface.h"
#include "WalletSingle.h"
#include "WalletLegacy/WalletLegacy.h"

using namespace Common;
using namespace crypto;
using namespace CryptoNote;

using namespace std;

// namespace
// {

// CryptoNote::WalletEvent makeSyncProgressUpdatedEvent(uint32_t current, uint32_t total)
// {
//   CryptoNote::WalletEvent event;
//   event.type = CryptoNote::WalletEventType::SYNC_PROGRESS_UPDATED;
//   event.synchronizationProgressUpdated.processedBlockCount = current;
//   event.synchronizationProgressUpdated.totalBlockCount = total;

//   return event;
// }

// CryptoNote::WalletEvent makeSyncCompletedEvent()
// {
//   CryptoNote::WalletEvent event;
//   event.type = CryptoNote::WalletEventType::SYNC_COMPLETED;

//   return event;
// }

// } // namespace

namespace MultiWalletService
{
// class SyncStarter : public CryptoNote::IWalletLegacyObserver
// {
// public:
//   SyncStarter(BlockchainSynchronizer &sync) : m_sync(sync) {}
//   virtual ~SyncStarter() {}

//   virtual void initCompleted(std::error_code result) override
//   {
//     if (!result)
//     {

//       m_sync.start();
//     }
//   }

//   BlockchainSynchronizer &m_sync;
// };

WalletInterface::WalletInterface(System::Dispatcher &dispatcher, const Currency &currency, INode &node, Logging::LoggerGroup &logger) : m_dispatcher(dispatcher),
                                                                                                                                        m_currency(currency),
                                                                                                                                        m_node(node),
                                                                                                                                        m_logger(logger)
// m_blockchainSynchronizerStarted(false),
// m_blockchainSynchronizer(node, currency.genesisBlockHash()),
// m_eventOccurred(m_dispatcher),
// m_onInitSyncStarter(new SyncStarter(m_blockchainSynchronizer)),
// m_readyEvent(m_dispatcher)
{
  // m_upperTransactionSizeLimit = m_currency.blockGrantedFullRewardZone() * 2 - m_currency.minerTxBlobReservedSize();
  // m_readyEvent.set();
}

WalletInterface::~WalletInterface()
{
  m_dispatcher.yield(); //let remote spawns finish
}

// void WalletInterface::init()
// {

//   m_blockchain.push_back(m_currency.genesisBlockHash());

//   m_blockchainSynchronizer.addObserver(this);
//   Logging::LoggerRef log(m_logger, "multi wallet service");

//   log(Logging::INFO) << "init wallet";
// }

// void WalletInterface::synchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount)
// {

//   Logging::LoggerRef log(m_logger, "synchronizationProgressUpdated");
//   log(Logging::INFO) << "wallet interface synchronized";
//   log(Logging::INFO) << "processedBlockCount" << processedBlockCount;
//   log(Logging::INFO) << "totalBlockCount" << totalBlockCount;
//   m_dispatcher.remoteSpawn([processedBlockCount, totalBlockCount, this]() { onSynchronizationProgressUpdated(processedBlockCount, totalBlockCount); });
// }

// void WalletInterface::synchronizationCompleted(std::error_code result)
// {
//   Logging::LoggerRef log(m_logger, "synchronizationCompleted");
//   log(Logging::INFO) << "wallet interface synchronized";
//   log(Logging::INFO) << result;

//   m_dispatcher.remoteSpawn([this]() { onSynchronizationCompleted(); });
// }

// void WalletInterface::onSynchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount)
// {
//   assert(processedBlockCount > 0);

//   Logging::LoggerRef log(m_logger, "onSynchronizationProgressUpdated");
//   log(Logging::INFO) << "processedBlockCount" << processedBlockCount;
//   log(Logging::INFO) << "totalBlockCount" << totalBlockCount;

//   System::EventLock lk(m_readyEvent);
//   pushEvent(makeSyncProgressUpdatedEvent(processedBlockCount, totalBlockCount));

//   currentHeight = processedBlockCount - 1;

//   Logging::LoggerRef(m_logger, "inteface")(Logging::INFO) << "on synchronizing" << endl;
// }

// void WalletInterface::onSynchronizationCompleted()
// {
//   System::EventLock lk(m_readyEvent);

//   Logging::LoggerRef(m_logger, "inteface")(Logging::INFO) << "on synchronized" << endl;

//   pushEvent(makeSyncCompletedEvent());
// }

// void WalletInterface::pushEvent(const WalletEvent &event)
// {
//   m_events.push(event);
//   m_eventOccurred.set();
// }

// void WalletInterface::startBlockchainSynchronizer()
// {
//   if (!m_blockchainSynchronizerStarted)
//   {
//     Logging::LoggerRef(m_logger, "startBlockchainSynchronizer")(Logging::INFO) << "started" << endl;

//     m_blockchainSynchronizer.start();
//     m_blockchainSynchronizerStarted = true;
//   }
// }

// void WalletInterface::stopBlockchainSynchronizer()
// {
//   if (m_blockchainSynchronizerStarted)
//   {
//     m_blockchainSynchronizer.stop();
//     m_blockchainSynchronizerStarted = false;
//   }
// }

// crypto::Hash WalletInterface::getBlockHashByIndex(uint32_t blockIndex) const
// {
//   assert(blockIndex < m_blockchain.size());
//   return m_blockchain.get<BlockHeightIndex>()[blockIndex];
// }

bool WalletInterface::createWallet(const AccountKeys &accountKeys)
{

  Logging::LoggerRef(m_logger, "inteface")(Logging::INFO) << "creating new wallet" << endl;
  CryptoNote::IWalletLegacy *wallet = new WalletSingle(m_currency, m_node, m_logger);
  // CryptoNote::IWalletLegacy *wallet = new WalletLegacy(m_currency, m_node);


  WalletHelper::InitWalletResultObserver initObserver;
  std::future<std::error_code> f_initError = initObserver.initResult.get_future();

  WalletHelper::IWalletRemoveObserverGuard removeGuard(*wallet, initObserver);

  wallet->initWithKeys(accountKeys, "");
  std::string address = getAddressesByKeys(accountKeys.address);
  m_wallets[address] = wallet;
  return true;
}

bool WalletInterface::isWalletExisted(const std::string &address)
{
  void *wallet = m_wallets[address];
  if (wallet)
  {
    return true;
  }
  return false;
}

bool WalletInterface::checkAddress(const std::string &address, AccountPublicAddress &keys)
{

  std::string genAddress = m_currency.accountAddressAsString(keys);

  if (genAddress != address)
  {
    return false;
  }
  return true;
}

CryptoNote::IWalletLegacy *WalletInterface::getWallet(const std::string address)
{
  std::map<std::string, void *>::iterator it;
  it = m_wallets.find(address);
  if (it != m_wallets.end())
  {
    return (CryptoNote::IWalletLegacy *)it->second;
  }
  return NULL;
}

std::string WalletInterface::getAddressesByKeys(const AccountPublicAddress &keys)
{
  return m_currency.accountAddressAsString(keys);
}

std::string WalletInterface::sha256(const std::string str)
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

} // namespace MultiWalletService
