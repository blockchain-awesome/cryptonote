// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include <unordered_map>

#include "cryptonote/core/Account.h"
#include "cryptonote/core/key.h"
#include "cryptonote/core/Currency.h"
#include "cryptonote/core/blockchain/indexing/exports.h"
#include "crypto/hash.h"

#include "../TestGenerator/TestGenerator.h"

class TestBlockchainGenerator
{
public:
  TestBlockchainGenerator(const cryptonote::Currency& currency);

  //TODO: get rid of this method
  std::vector<cryptonote::block_t>& getBlockchain();
  std::vector<cryptonote::block_t> getBlockchainCopy();
  void generateEmptyBlocks(size_t count);
  bool getBlockRewardForAddress(const cryptonote::AccountPublicAddress& address);
  bool generateTransactionsInOneBlock(const cryptonote::AccountPublicAddress& address, size_t n);
  bool getSingleOutputTransaction(const cryptonote::AccountPublicAddress& address, uint64_t amount);
  void addTxToBlockchain(const cryptonote::Transaction& transaction);
  bool getTransactionByHash(const crypto::Hash& hash, cryptonote::Transaction& tx, bool checkTxPool = false);
  const cryptonote::AccountBase& getMinerAccount() const;
  bool generateFromBaseTx(const cryptonote::AccountBase& address);

  void putTxToPool(const cryptonote::Transaction& tx);
  void getPoolSymmetricDifference(std::vector<crypto::Hash>&& known_pool_tx_ids, crypto::Hash known_block_id, bool& is_bc_actual,
    std::vector<cryptonote::Transaction>& new_txs, std::vector<crypto::Hash>& deleted_tx_ids);
  void putTxPoolToBlockchain();
  void clearTxPool();

  void cutBlockchain(uint32_t height);

  bool addOrphan(const crypto::Hash& hash, uint32_t height);
  bool getGeneratedTransactionsNumber(uint32_t height, uint64_t& generatedTransactions);
  bool getOrphanBlockIdsByHeight(uint32_t height, std::vector<crypto::Hash>& blockHashes);
  bool getBlockIdsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<crypto::Hash>& hashes, uint32_t& blocksNumberWithinTimestamps);
  bool getPoolTransactionIdsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<crypto::Hash>& hashes, uint64_t& transactionsNumberWithinTimestamps);
  bool getTransactionIdsByPaymentId(const crypto::Hash& paymentId, std::vector<crypto::Hash>& transactionHashes);

  bool getTransactionGlobalIndexesByHash(const crypto::Hash& transactionHash, std::vector<uint32_t>& globalIndexes);
  bool getMultisignatureOutputByGlobalIndex(uint64_t amount, uint32_t globalIndex, cryptonote::MultisignatureOutput& out);
  void setMinerAccount(const cryptonote::AccountBase& account);

private:
  struct MultisignatureOutEntry {
    crypto::Hash transactionHash;
    uint16_t indexOut;
  };

  struct KeyOutEntry {
    crypto::Hash transactionHash;
    uint16_t indexOut;
  };
  
  void addGenesisBlock();
  void addMiningBlock();

  const cryptonote::Currency& m_currency;
  test_generator generator;
  cryptonote::AccountBase miner_acc;
  std::vector<cryptonote::block_t> m_blockchain;
  std::unordered_map<crypto::Hash, cryptonote::Transaction> m_txs;
  std::unordered_map<crypto::Hash, std::vector<uint32_t>> transactionGlobalOuts;
  std::unordered_map<uint64_t, std::vector<MultisignatureOutEntry>> multisignatureOutsIndex;
  std::unordered_map<uint64_t, std::vector<KeyOutEntry>> keyOutsIndex;

  std::unordered_map<crypto::Hash, cryptonote::Transaction> m_txPool;
  mutable std::mutex m_mutex;

  cryptonote::PaymentIdIndex m_paymentIdIndex;
  cryptonote::TimestampTransactionsIndex m_timestampIndex;
  cryptonote::GeneratedTransactionsIndex m_generatedTransactionsIndex;
  cryptonote::OrphanBlocksIndex m_orthanBlocksIndex;

  void addToBlockchain(const cryptonote::Transaction& tx);
  void addToBlockchain(const std::vector<cryptonote::Transaction>& txs);
  void addToBlockchain(const std::vector<cryptonote::Transaction>& txs, const cryptonote::AccountBase& minerAddress);
  void addTx(const cryptonote::Transaction& tx);

  bool doGenerateTransactionsInOneBlock(cryptonote::AccountPublicAddress const &address, size_t n);
};
