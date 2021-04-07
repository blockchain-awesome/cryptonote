// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <mutex>
#include <atomic>
#include <unordered_set>

#include "IBlockchainExplorer.h"
#include "INode.h"

#include "common/ObserverManager.h"
#include "BlockchainExplorerErrors.h"

#include "wallet/WalletAsyncContextCounter.h"

#include "logging/LoggerRef.h"

namespace cryptonote {

class BlockchainExplorer : public IBlockchainExplorer, public INodeObserver {
public:
  BlockchainExplorer(INode& node, Logging::ILogger& logger);

  BlockchainExplorer(const BlockchainExplorer&) = delete;
  BlockchainExplorer(BlockchainExplorer&&) = delete;

  BlockchainExplorer& operator=(const BlockchainExplorer&) = delete;
  BlockchainExplorer& operator=(BlockchainExplorer&&) = delete;

  virtual ~BlockchainExplorer();
    
  virtual bool addObserver(IBlockchainObserver* observer) override;
  virtual bool removeObserver(IBlockchainObserver* observer) override;

  virtual bool getBlocks(const std::vector<uint32_t>& blockHeights, std::vector<std::vector<block_details_t>>& blocks) override;
  virtual bool getBlocks(const std::vector<hash_t>& blockHashes, std::vector<block_details_t>& blocks) override;
  virtual bool getBlocks(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<block_details_t>& blocks, uint32_t& blocksNumberWithinTimestamps) override;

  virtual bool getBlockchainTop(block_details_t& topBlock) override;

  virtual bool getTransactions(const std::vector<hash_t>& transactionHashes, std::vector<transaction_explorer_details_t>& transactions) override;
  virtual bool getTransactionsByPaymentId(const hash_t& paymentId, std::vector<transaction_explorer_details_t>& transactions) override;
  virtual bool getPoolTransactions(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<transaction_explorer_details_t>& transactions, uint64_t& transactionsNumberWithinTimestamps) override;
  virtual bool getPoolState(const std::vector<hash_t>& knownPoolTransactionHashes, hash_t knownBlockchainTop, bool& isBlockchainActual, std::vector<transaction_explorer_details_t>& newTransactions, std::vector<hash_t>& removedTransactions) override;

  virtual uint64_t getRewardBlocksWindow() override;
  virtual uint64_t getFullRewardMaxBlockSize(uint8_t majorVersion) override;

  virtual bool isSynchronized() override;

  virtual void init() override;
  virtual void shutdown() override;

  virtual void poolChanged() override;
  virtual void blockchainSynchronized(uint32_t topHeight) override;
  virtual void localBlockchainUpdated(uint32_t height) override;

  typedef WalletAsyncContextCounter AsyncContextCounter;

private:
  void poolUpdateEndHandler();

  class PoolUpdateGuard {
  public:
    PoolUpdateGuard();

    bool beginUpdate();
    bool endUpdate();

  private:
    enum class State {
      NONE,
      UPDATING,
      UPDATE_REQUIRED
    };

    std::atomic<State> m_state;
  };

  enum State {
    NOT_INITIALIZED,
    INITIALIZED
  };

  block_details_t knownBlockchainTop;
  uint32_t knownBlockchainTopHeight;
  std::unordered_set<hash_t> knownPoolState;

  std::atomic<State> state;
  std::atomic<bool> synchronized;
  std::atomic<uint32_t> observersCounter;
  Tools::ObserverManager<IBlockchainObserver> observerManager;

  std::mutex mutex;

  INode& node;
  Logging::LoggerRef logger;

  AsyncContextCounter asyncContextCounter;
  PoolUpdateGuard poolUpdateGuard;
};
}
