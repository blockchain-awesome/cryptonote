// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include <unordered_map>

#include "cryptonote/core/account.h"
#include "cryptonote/core/key.h"
#include "cryptonote/core/currency.h"
#include "cryptonote/core/blockchain/indexing/exports.h"
#include "cryptonote/crypto/hash.h"

#include "../TestGenerator/TestGenerator.h"

class TestBlockchainGenerator
{
public:
  TestBlockchainGenerator(const cryptonote::Currency& currency);

  //TODO: get rid of this method
  std::vector<cryptonote::block_t>& getBlockchain();
  std::vector<cryptonote::block_t> getBlockchainCopy();
  void generateEmptyBlocks(size_t count);
  bool getBlockRewardForAddress(const cryptonote::account_public_address_t& address);
  bool generateTransactionsInOneBlock(const cryptonote::account_public_address_t& address, size_t n);
  bool getSingleOutputTransaction(const cryptonote::account_public_address_t& address, uint64_t amount);
  void addTxToBlockchain(const cryptonote::transaction_t& transaction);
  bool getTransactionByHash(const hash_t& hash, cryptonote::transaction_t& tx, bool checkTxPool = false);
  const cryptonote::Account& getMinerAccount() const;
  bool generateFromBaseTx(const cryptonote::Account& address);

  void putTxToPool(const cryptonote::transaction_t& tx);
  void getPoolSymmetricDifference(std::vector<hash_t>&& known_pool_tx_ids, hash_t known_block_id, bool& is_bc_actual,
    std::vector<cryptonote::transaction_t>& new_txs, std::vector<hash_t>& deleted_tx_ids);
  void putTxPoolToBlockchain();
  void clearTxPool();

  void cutBlockchain(uint32_t height);

  bool addOrphan(const hash_t& hash, uint32_t height);
  bool getGeneratedTransactionsNumber(uint32_t height, uint64_t& generatedTransactions);
  bool getOrphanBlockIdsByHeight(uint32_t height, std::vector<hash_t>& blockHashes);
  bool getBlockIdsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<hash_t>& hashes, uint32_t& blocksNumberWithinTimestamps);
  bool getPoolTransactionIdsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<hash_t>& hashes, uint64_t& transactionsNumberWithinTimestamps);
  bool getTransactionIdsByPaymentId(const hash_t& paymentId, std::vector<hash_t>& transactionHashes);

  bool getTransactionGlobalIndexesByHash(const hash_t& transactionHash, std::vector<uint32_t>& globalIndexes);
  bool getMultisignatureOutputByGlobalIndex(uint64_t amount, uint32_t globalIndex, cryptonote::multi_signature_output_t& out);
  void setMinerAccount(const cryptonote::Account& account);

private:
  struct MultisignatureOutEntry {
    hash_t transactionHash;
    uint16_t indexOut;
  };

  struct KeyOutEntry {
    hash_t transactionHash;
    uint16_t indexOut;
  };
  
  void addGenesisBlock();
  void addMiningBlock();

  const cryptonote::Currency& m_currency;
  test_generator generator;
  cryptonote::Account miner_acc;
  std::vector<cryptonote::block_t> m_blockchain;
  std::unordered_map<hash_t, cryptonote::transaction_t> m_txs;
  std::unordered_map<hash_t, std::vector<uint32_t>> transactionGlobalOuts;
  std::unordered_map<uint64_t, std::vector<MultisignatureOutEntry>> multisignatureOutsIndex;
  std::unordered_map<uint64_t, std::vector<KeyOutEntry>> keyOutsIndex;

  std::unordered_map<hash_t, cryptonote::transaction_t> m_txPool;
  mutable std::mutex m_mutex;

  cryptonote::PaymentIdIndex m_paymentIdIndex;
  cryptonote::TimestampTransactionsIndex m_timestampIndex;
  cryptonote::GeneratedTransactionsIndex m_generatedTransactionsIndex;
  cryptonote::OrphanBlocksIndex m_orthanBlocksIndex;

  void addToBlockchain(const cryptonote::transaction_t& tx);
  void addToBlockchain(const std::vector<cryptonote::transaction_t>& txs);
  void addToBlockchain(const std::vector<cryptonote::transaction_t>& txs, const cryptonote::Account& minerAddress);
  void addTx(const cryptonote::transaction_t& tx);

  bool doGenerateTransactionsInOneBlock(cryptonote::account_public_address_t const &address, size_t n);
};
