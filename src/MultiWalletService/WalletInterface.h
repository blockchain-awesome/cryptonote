// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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

#include "logging/LoggerGroup.h"
// #include "WalletSingle.h"
// #include "WalletInterface.h"

using namespace cryptonote;

namespace MultiWalletService
{

// class SyncStarter;

class WalletInterface
{
public:
  WalletInterface(System::Dispatcher &dispatcher, const Currency &currency, INode &node, Logging::LoggerGroup &logger);
  virtual ~WalletInterface();
  bool createWallet(const AccountKeys &accountKeys);
  void init();

  bool isWalletExisted(const std::string &address);

  bool checkAddress(const std::string &address, AccountPublicAddress &keys);

  cryptonote::IWalletLegacy *getWallet(const std::string token);

  std::string getAddressesByKeys(const AccountPublicAddress &keys);

  std::string sha256(const std::string str);

protected:
  // 区块链同步接口

  // void startBlockchainSynchronizer();
  // virtual void synchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount) override;
  // virtual void synchronizationCompleted(std::error_code result) override;
  // void onSynchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount);
  // void onSynchronizationCompleted();

  // void stopBlockchainSynchronizer();

  // void pushEvent(const WalletEvent &event);

  // crypto::Hash getBlockHashByIndex(uint32_t blockIndex) const;

  // bool m_blockchainSynchronizerStarted;
  // BlockchainSynchronizer m_blockchainSynchronizer;
  // std::unique_ptr<SyncStarter> m_onInitSyncStarter;

  Logging::LoggerGroup &m_logger;

  // cryptonote::AccountBase m_account;

  System::Dispatcher &m_dispatcher;
  const Currency &m_currency;
  INode &m_node;
  // bool m_stopped;

  // System::Event m_eventOccurred;
  // std::queue<WalletEvent> m_events;
  // mutable System::Event m_readyEvent;

  // uint64_t m_upperTransactionSizeLimit;
  // uint32_t m_transactionSoftLockTime;

  // BlockHashesContainer m_blockchain;

  std::map<std::string, void *> m_wallets;

  uint32_t currentHeight;
};

} // namespace MultiWalletService
