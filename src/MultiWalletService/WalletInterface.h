// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "IWallet.h"

#include <queue>
#include <map>
#include <string>

#include "Wallet/IFusionManager.h"
#include "Wallet/WalletIndices.h"

#include <System/Dispatcher.h>
#include <System/Event.h>
#include "Transfers/TransfersSynchronizer.h"
#include "Transfers/BlockchainSynchronizer.h"

#include "Logging/LoggerGroup.h"
#include "WalletSingle.h"

using namespace CryptoNote;

namespace MultiWalletService
{

class WalletInterface : IBlockchainSynchronizerObserver
{
public:
  WalletInterface(System::Dispatcher &dispatcher, const Currency &currency, INode &node, Logging::LoggerGroup &logger, uint32_t transactionSoftLockTime = 1);
  virtual ~WalletInterface();

protected:
  // 区块链同步接口

  void startBlockchainSynchronizer();
  virtual void synchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount) override;
  virtual void synchronizationCompleted(std::error_code result) override;
  void onSynchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount);
  void onSynchronizationCompleted();

  void stopBlockchainSynchronizer();

  void init();

  void pushEvent(const WalletEvent &event);

  void createWallet(const AccountKeys &accountKeys);

  Crypto::Hash getBlockHashByIndex(uint32_t blockIndex) const;

  bool m_blockchainSynchronizerStarted;
  BlockchainSynchronizer m_blockchainSynchronizer;

  Logging::LoggerGroup &m_logger;

  System::Dispatcher &m_dispatcher;
  const Currency &m_currency;
  INode &m_node;
  bool m_stopped;

  TransfersSyncronizer m_synchronizer;

  System::Event m_eventOccurred;
  std::queue<WalletEvent> m_events;
  mutable System::Event m_readyEvent;

  uint64_t m_upperTransactionSizeLimit;
  uint32_t m_transactionSoftLockTime;

  BlockHashesContainer m_blockchain;

  std::map<std::string, void*> m_wallets;
};

} // namespace MultiWalletService
