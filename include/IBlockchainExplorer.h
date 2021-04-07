// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include <array>

#include "BlockchainExplorerData.h"

namespace cryptonote {

class IBlockchainObserver {
public:
  virtual ~IBlockchainObserver() {}

  virtual void blockchainUpdated(const std::vector<block_details_t>& newBlocks, const std::vector<block_details_t>& orphanedBlocks) {}
  virtual void poolUpdated(const std::vector<transaction_explorer_details_t>& newTransactions, const std::vector<std::pair<hash_t, transaction_remove_reason_t>>& removedTransactions) {}

  virtual void blockchainSynchronized(const block_details_t& topBlock) {}
};

class IBlockchainExplorer {
public:
  virtual ~IBlockchainExplorer() {};

  virtual bool addObserver(IBlockchainObserver* observer) = 0;
  virtual bool removeObserver(IBlockchainObserver* observer) = 0;

  virtual void init() = 0;
  virtual void shutdown() = 0;

  virtual bool getBlocks(const std::vector<uint32_t>& blockHeights, std::vector<std::vector<block_details_t>>& blocks) = 0;
  virtual bool getBlocks(const std::vector<hash_t>& blockHashes, std::vector<block_details_t>& blocks) = 0;
  virtual bool getBlocks(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<block_details_t>& blocks, uint32_t& blocksNumberWithinTimestamps) = 0;

  virtual bool getBlockchainTop(block_details_t& topBlock) = 0;

  virtual bool getTransactions(const std::vector<hash_t>& transactionHashes, std::vector<transaction_explorer_details_t>& transactions) = 0;
  virtual bool getTransactionsByPaymentId(const hash_t& paymentId, std::vector<transaction_explorer_details_t>& transactions) = 0;
  virtual bool getPoolTransactions(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<transaction_explorer_details_t>& transactions, uint64_t& transactionsNumberWithinTimestamps) = 0;
  virtual bool getPoolState(const std::vector<hash_t>& knownPoolTransactionHashes, hash_t knownBlockchainTop, bool& isBlockchainActual, std::vector<transaction_explorer_details_t>& newTransactions, std::vector<hash_t>& removedTransactions) = 0;

  virtual uint64_t getRewardBlocksWindow() = 0;
  virtual uint64_t getFullRewardMaxBlockSize(uint8_t majorVersion) = 0;

  virtual bool isSynchronized() = 0;
};

}
