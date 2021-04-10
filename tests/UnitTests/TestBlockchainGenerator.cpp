// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TestBlockchainGenerator.h"

#include <time.h>
#include <unordered_set>

#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/structures/block_entry.h"
#include "stream/block.h"
#include "stream/transaction.h"
#include "cryptonote/structures/array.hpp"

#include "../PerformanceTests/MultiTransactionTestBase.h"

using namespace cryptonote;

class TransactionForAddressCreator : public multi_tx_test_base<5>
{
  typedef multi_tx_test_base<5> base_class;
public:
  TransactionForAddressCreator() {}

  bool init()
  {
    return base_class::init();
  }

  void generate(const account_public_address_t& address, transaction_t& tx, uint64_t unlockTime = 0)
  {
    std::vector<cryptonote::transaction_destination_entry_t> destinations;

    cryptonote::decompose_amount_into_digits(this->m_source_amount, 0,
      [&](uint64_t chunk) { destinations.push_back(cryptonote::transaction_destination_entry_t(chunk, address)); },
      [&](uint64_t a_dust) { destinations.push_back(cryptonote::transaction_destination_entry_t(a_dust, address)); });

    cryptonote::constructTransaction(this->m_miners[this->real_source_idx].getAccountKeys(), this->m_sources, destinations, std::vector<uint8_t>(), tx, unlockTime, m_logger);
  }

  void generateSingleOutputTx(const account_public_address_t& address, uint64_t amount, transaction_t& tx) {
    std::vector<transaction_destination_entry_t> destinations;
    destinations.push_back(transaction_destination_entry_t(amount, address));
    constructTransaction(this->m_miners[this->real_source_idx].getAccountKeys(), this->m_sources, destinations, std::vector<uint8_t>(), tx, 0, m_logger);
  }
};

TestBlockchainGenerator::TestBlockchainGenerator(const cryptonote::Currency& currency) :
  m_currency(currency),
  generator(currency)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  miner_acc.generate();
  addGenesisBlock();
  addMiningBlock();
}

std::vector<cryptonote::block_t>& TestBlockchainGenerator::getBlockchain()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  return m_blockchain;
}

std::vector<cryptonote::block_t> TestBlockchainGenerator::getBlockchainCopy() {
  std::unique_lock<std::mutex> lock(m_mutex);

  std::vector<cryptonote::block_t> blockchain(m_blockchain);
  return blockchain;
}

bool TestBlockchainGenerator::getTransactionByHash(const hash_t& hash, cryptonote::transaction_t& tx, bool checkTxPool)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  auto it = m_txs.find(hash);
  if (it != m_txs.end()) {
    tx = it->second;
    return true;
  } else if (checkTxPool) {
    auto poolIt = m_txPool.find(hash);
    if (poolIt != m_txPool.end()) {
      tx = poolIt->second;
      return true;
    }
  }
  return false;
}

const cryptonote::Account& TestBlockchainGenerator::getMinerAccount() const {
  std::unique_lock<std::mutex> lock(m_mutex);
  return miner_acc;
}

void TestBlockchainGenerator::addGenesisBlock() {
  std::vector<size_t> bsizes;
  generator.addBlock(m_currency.genesisBlock(), 0, 0, bsizes, 0);

  m_blockchain.push_back(m_currency.genesisBlock());
  addTx(m_currency.genesisBlock().baseTransaction);

  m_timestampIndex.add(m_currency.genesisBlock().timestamp, cryptonote::Block::getHash(m_currency.genesisBlock()));
  m_generatedTransactionsIndex.add(m_currency.genesisBlock());
}

void TestBlockchainGenerator::addMiningBlock() {
  cryptonote::block_t block;

  uint64_t timestamp = time(NULL);
  cryptonote::block_t& prev_block = m_blockchain.back();
  uint32_t height = boost::get<base_input_t>(prev_block.baseTransaction.inputs.front()).blockIndex + 1;
  hash_t prev_id = Block::getHash(prev_block);

  std::vector<size_t> block_sizes;
  std::list<cryptonote::transaction_t> tx_list;

  generator.constructBlock(block, height, prev_id, miner_acc, timestamp, 0, block_sizes, tx_list);
  m_blockchain.push_back(block);
  addTx(block.baseTransaction);

  m_timestampIndex.add(block.timestamp, cryptonote::Block::getHash(block));
  m_generatedTransactionsIndex.add(block);
}

void TestBlockchainGenerator::generateEmptyBlocks(size_t count)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  for (size_t i = 0; i < count; ++i)
  {
    cryptonote::block_t& prev_block = m_blockchain.back();
    cryptonote::block_t block;
    generator.constructBlock(block, prev_block, miner_acc);
    m_blockchain.push_back(block);
    addTx(block.baseTransaction);

    m_timestampIndex.add(block.timestamp, cryptonote::Block::getHash(block));
    m_generatedTransactionsIndex.add(block);
  }
}

void TestBlockchainGenerator::addTxToBlockchain(const cryptonote::transaction_t& transaction)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  addToBlockchain(transaction);
}

bool TestBlockchainGenerator::getBlockRewardForAddress(const cryptonote::account_public_address_t& address)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  doGenerateTransactionsInOneBlock(address, 1);
  return true;
}

bool TestBlockchainGenerator::generateTransactionsInOneBlock(const cryptonote::account_public_address_t& address, size_t n) {
  std::unique_lock<std::mutex> lock(m_mutex);

  return doGenerateTransactionsInOneBlock(address, n);
}

bool TestBlockchainGenerator::doGenerateTransactionsInOneBlock(const account_public_address_t &address, size_t n) {
  assert(n > 0);

  TransactionForAddressCreator creator;
  if (!creator.init())
    return false;

  std::vector<transaction_t> txs;
  for (size_t i = 0; i < n; ++i) {
    transaction_t tx;
    creator.generate(address, tx, m_blockchain.size() + 10);
    txs.push_back(tx);
  }

  addToBlockchain(txs);

  return true;
}

bool TestBlockchainGenerator::getSingleOutputTransaction(const cryptonote::account_public_address_t& address, uint64_t amount) {
  std::unique_lock<std::mutex> lock(m_mutex);

  TransactionForAddressCreator creator;
  if (!creator.init())
    return false;

  cryptonote::transaction_t tx;
  creator.generateSingleOutputTx(address, amount, tx);

  addToBlockchain(tx);

  return true;
}

void TestBlockchainGenerator::addToBlockchain(const cryptonote::transaction_t& tx) {
  addToBlockchain(std::vector<cryptonote::transaction_t> {tx});
}

void TestBlockchainGenerator::addToBlockchain(const std::vector<cryptonote::transaction_t>& txs) {
  addToBlockchain(txs, miner_acc);
}

void TestBlockchainGenerator::addToBlockchain(const std::vector<cryptonote::transaction_t>& txs, const cryptonote::Account& minerAddress) {
  std::list<cryptonote::transaction_t> txsToBlock;

  for (const auto& tx: txs) {
    addTx(tx);

    txsToBlock.push_back(tx);
    m_paymentIdIndex.add(tx);
  }

  cryptonote::block_t& prev_block = m_blockchain.back();
  cryptonote::block_t block;

  generator.constructBlock(block, prev_block, minerAddress, txsToBlock);
  m_blockchain.push_back(block);
  addTx(block.baseTransaction);

  m_timestampIndex.add(block.timestamp, cryptonote::Block::getHash(block));
  m_generatedTransactionsIndex.add(block);
}

void TestBlockchainGenerator::getPoolSymmetricDifference(std::vector<hash_t>&& known_pool_tx_ids, hash_t known_block_id, bool& is_bc_actual,
  std::vector<cryptonote::transaction_t>& new_txs, std::vector<hash_t>& deleted_tx_ids)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  if (known_block_id != cryptonote::Block::getHash(m_blockchain.back())) {
    is_bc_actual = false;
    return;
  }

  is_bc_actual = true;

  std::unordered_set<hash_t> txIds;
  for (const auto& kv : m_txPool) {
    txIds.insert(kv.first);
  }

  std::unordered_set<hash_t> known_set(known_pool_tx_ids.begin(), known_pool_tx_ids.end());
  for (auto it = txIds.begin(), e = txIds.end(); it != e;) {
    auto known_it = known_set.find(*it);
    if (known_it != known_set.end()) {
      known_set.erase(known_it);
      it = txIds.erase(it);
    }
    else {
      new_txs.push_back(m_txPool[*it]);
      ++it;
    }
  }

  deleted_tx_ids.assign(known_set.begin(), known_set.end());
}

void TestBlockchainGenerator::putTxToPool(const cryptonote::transaction_t& tx) {
  std::unique_lock<std::mutex> lock(m_mutex);

  hash_t txHash = cryptonote::BinaryArray::objectHash(tx);
  m_txPool[txHash] = tx;
}

void TestBlockchainGenerator::putTxPoolToBlockchain() {
  std::unique_lock<std::mutex> lock(m_mutex);
  std::vector<cryptonote::transaction_t> txs;
  for (auto& kv : m_txPool) {
    txs.push_back(kv.second);
  }

  addToBlockchain(txs);
  m_txPool.clear();
}

void TestBlockchainGenerator::clearTxPool() {
  std::unique_lock<std::mutex> lock(m_mutex);

  m_txPool.clear();
}

void TestBlockchainGenerator::cutBlockchain(uint32_t height) {
  std::unique_lock<std::mutex> lock(m_mutex);

  assert(height < m_blockchain.size());
  //assert(height > m_lastHeight);

  auto it = m_blockchain.begin();
  std::advance(it, height);

  m_blockchain.erase(it, m_blockchain.end());

  //TODO: delete transactions from m_txs
}

bool TestBlockchainGenerator::addOrphan(const hash_t& hash, uint32_t height) {
  cryptonote::block_t block;
  uint64_t timestamp = time(NULL);
  generator.constructBlock(block, miner_acc, timestamp);
  return m_orthanBlocksIndex.add(block);
}

void TestBlockchainGenerator::setMinerAccount(const cryptonote::Account& account) {
  miner_acc = account;
}

bool TestBlockchainGenerator::getGeneratedTransactionsNumber(uint32_t height, uint64_t& generatedTransactions) {
  return m_generatedTransactionsIndex.find(height, generatedTransactions);
}

bool TestBlockchainGenerator::getOrphanBlockIdsByHeight(uint32_t height, std::vector<hash_t>& blockHashes) {
  return m_orthanBlocksIndex.find(height, blockHashes);
}

bool TestBlockchainGenerator::getBlockIdsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<hash_t>& hashes, uint32_t& blocksNumberWithinTimestamps) {
  uint64_t blockCount;
  if (!m_timestampIndex.find(timestampBegin, timestampEnd, blocksNumberLimit, hashes, blockCount)) {
    return false;
  }

  blocksNumberWithinTimestamps = static_cast<uint32_t>(blockCount);
  return true;
}

bool TestBlockchainGenerator::getPoolTransactionIdsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<hash_t>& hashes, uint64_t& transactionsNumberWithinTimestamps) {
  std::vector<hash_t> blockHashes;
  if (!m_timestampIndex.find(timestampBegin, timestampEnd, transactionsNumberLimit, blockHashes, transactionsNumberWithinTimestamps)) {
    return false;
  }
  transactionsNumberWithinTimestamps = m_txPool.size();
  uint32_t c = 0;
  for (auto i : m_txPool) {
    if (c >= transactionsNumberLimit) {
      return true;
    }
    hashes.push_back(cryptonote::BinaryArray::objectHash(i.second));
    ++c;
  }
  return true;
}

bool TestBlockchainGenerator::getTransactionIdsByPaymentId(const hash_t& paymentId, std::vector<hash_t>& transactionHashes) {
  return m_paymentIdIndex.find(paymentId, transactionHashes);
}

void TestBlockchainGenerator::addTx(const cryptonote::transaction_t& tx) {
  hash_t txHash = BinaryArray::objectHash(tx);
  m_txs[txHash] = tx;
  auto& globalIndexes = transactionGlobalOuts[txHash];
  for (uint16_t outIndex = 0; outIndex < tx.outputs.size(); ++outIndex) {
    const auto& out = tx.outputs[outIndex];
    if (out.target.type() == typeid(key_output_t)) {
      auto& keyOutsContainer = keyOutsIndex[out.amount];
      globalIndexes.push_back(static_cast<uint32_t>(keyOutsContainer.size()));
      keyOutsContainer.push_back({ txHash, outIndex });
    } else if (out.target.type() == typeid(multi_signature_output_t)) {
      auto& msigOutsContainer = multisignatureOutsIndex[out.amount];
      globalIndexes.push_back(static_cast<uint32_t>(msigOutsContainer.size()));
      msigOutsContainer.push_back({ txHash, outIndex });
    }
  }
}

bool TestBlockchainGenerator::getTransactionGlobalIndexesByHash(const hash_t& transactionHash, std::vector<uint32_t>& globalIndexes) {
  auto globalIndexesIt = transactionGlobalOuts.find(transactionHash);
  if (globalIndexesIt == transactionGlobalOuts.end()) {
    return false;
  }

  globalIndexes = globalIndexesIt->second;
  return true;
}

bool TestBlockchainGenerator::getMultisignatureOutputByGlobalIndex(uint64_t amount, uint32_t globalIndex, multi_signature_output_t& out) {
  auto it = multisignatureOutsIndex.find(amount);
  if (it == multisignatureOutsIndex.end()) {
    return false;
  }

  if (it->second.size() <= globalIndex) {
    return false;
  }

  MultisignatureOutEntry entry = it->second[globalIndex];
  const auto& tx = m_txs[entry.transactionHash];
  assert(tx.outputs.size() > entry.indexOut);
  assert(tx.outputs[entry.indexOut].target.type() == typeid(multi_signature_output_t));
  out = boost::get<multi_signature_output_t>(tx.outputs[entry.indexOut].target);
  return true;
}

bool TestBlockchainGenerator::generateFromBaseTx(const cryptonote::Account& address) {
  std::unique_lock<std::mutex> lock(m_mutex);
  addToBlockchain({}, address);
  return true;
}
