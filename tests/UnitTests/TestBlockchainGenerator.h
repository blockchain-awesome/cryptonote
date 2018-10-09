// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include <unordered_map>

#include "cryptonote_core/Account.h"
#include "cryptonote_core/CryptoNoteBasic.h"
#include "cryptonote_core/Currency.h"
#include "cryptonote_core/BlockchainIndices.h"
#include "crypto/hash.h"

#include "../TestGenerator/TestGenerator.h"

class TestBlockchainGenerator
{
public:
  TestBlockchainGenerator(const CryptoNote::Currency& currency);

  //TODO: get rid of this method
  std::vector<CryptoNote::Block>& getBlockchain();
  std::vector<CryptoNote::Block> getBlockchainCopy();
  void generateEmptyBlocks(size_t count);
  bool getBlockRewardForAddress(const CryptoNote::AccountPublicAddress& address);
  bool generateTransactionsInOneBlock(const CryptoNote::AccountPublicAddress& address, size_t n);
  bool getSingleOutputTransaction(const CryptoNote::AccountPublicAddress& address, uint64_t amount);
  void addTxToBlockchain(const CryptoNote::Transaction& transaction);
  bool getTransactionByHash(const crypto::Hash& hash, CryptoNote::Transaction& tx, bool checkTxPool = false);
  const CryptoNote::AccountBase& getMinerAccount() const;
  bool generateFromBaseTx(const CryptoNote::AccountBase& address);

  void putTxToPool(const CryptoNote::Transaction& tx);
  void getPoolSymmetricDifference(std::vector<crypto::Hash>&& known_pool_tx_ids, crypto::Hash known_block_id, bool& is_bc_actual,
    std::vector<CryptoNote::Transaction>& new_txs, std::vector<crypto::Hash>& deleted_tx_ids);
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
  bool getMultisignatureOutputByGlobalIndex(uint64_t amount, uint32_t globalIndex, CryptoNote::MultisignatureOutput& out);
  void setMinerAccount(const CryptoNote::AccountBase& account);

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

  const CryptoNote::Currency& m_currency;
  test_generator generator;
  CryptoNote::AccountBase miner_acc;
  std::vector<CryptoNote::Block> m_blockchain;
  std::unordered_map<crypto::Hash, CryptoNote::Transaction> m_txs;
  std::unordered_map<crypto::Hash, std::vector<uint32_t>> transactionGlobalOuts;
  std::unordered_map<uint64_t, std::vector<MultisignatureOutEntry>> multisignatureOutsIndex;
  std::unordered_map<uint64_t, std::vector<KeyOutEntry>> keyOutsIndex;

  std::unordered_map<crypto::Hash, CryptoNote::Transaction> m_txPool;
  mutable std::mutex m_mutex;

  CryptoNote::PaymentIdIndex m_paymentIdIndex;
  CryptoNote::TimestampTransactionsIndex m_timestampIndex;
  CryptoNote::GeneratedTransactionsIndex m_generatedTransactionsIndex;
  CryptoNote::OrphanBlocksIndex m_orthanBlocksIndex;

  void addToBlockchain(const CryptoNote::Transaction& tx);
  void addToBlockchain(const std::vector<CryptoNote::Transaction>& txs);
  void addToBlockchain(const std::vector<CryptoNote::Transaction>& txs, const CryptoNote::AccountBase& minerAddress);
  void addTx(const CryptoNote::Transaction& tx);

  bool doGenerateTransactionsInOneBlock(CryptoNote::AccountPublicAddress const &address, size_t n);
};
