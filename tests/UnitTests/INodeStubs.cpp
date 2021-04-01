// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "INodeStubs.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/structures/array.hpp"
#include "cryptonote/core/transaction/TransactionApi.h"
#include "wallet/WalletErrors.h"

#include <functional>
#include <thread>
#include <iterator>
#include <cassert>
#include <unordered_set>

#include <system_error>

#include "cryptonote/crypto/crypto.h"
#include "cryptonote/structures/block_entry.h"

#include "blockchain_explorer/BlockchainExplorerDataBuilder.h"

using namespace cryptonote;
using namespace Common;

namespace {

class ContextCounterHolder {
public:
  ContextCounterHolder(WalletAsyncContextCounter& shutdowner) : m_shutdowner(shutdowner) {}
  ~ContextCounterHolder() { m_shutdowner.delAsyncContext(); }

private:
  WalletAsyncContextCounter& m_shutdowner;
};

}


void INodeDummyStub::updateObservers() {
  observerManager.notify(&INodeObserver::lastKnownBlockHeightUpdated, getLastKnownBlockHeight());
}

bool INodeDummyStub::addObserver(INodeObserver* observer) {
  return observerManager.add(observer);
}

bool INodeDummyStub::removeObserver(INodeObserver* observer) {
  return observerManager.remove(observer);
}

void INodeTrivialRefreshStub::getNewBlocks(std::vector<hash_t>&& knownBlockIds, std::vector<block_complete_entry_t>& newBlocks, uint32_t& startHeight, const Callback& callback)
{
  m_asyncCounter.addAsyncContext();

  std::unique_lock<std::mutex> lock(m_walletLock);
  auto blockchain = m_blockchainGenerator.getBlockchainCopy();
  lock.unlock();

  std::thread task(std::bind(&INodeTrivialRefreshStub::doGetNewBlocks, this, std::move(knownBlockIds), std::ref(newBlocks),
          std::ref(startHeight), std::move(blockchain), callback));
  task.detach();
}

void INodeTrivialRefreshStub::waitForAsyncContexts() {
  m_asyncCounter.waitAsyncContextsFinish();
}

void INodeTrivialRefreshStub::doGetNewBlocks(std::vector<hash_t> knownBlockIds, std::vector<block_complete_entry_t>& newBlocks,
        uint32_t& startHeight, std::vector<block_t> blockchain, const Callback& callback)
{
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  std::vector<block_t>::iterator start = blockchain.end();

  for (const auto& id : knownBlockIds) {
    start = std::find_if(blockchain.begin(), blockchain.end(), 
      [&id](block_t& block) { return Block::getHash(block) == id; });
    if (start != blockchain.end())
      break;
  }

  if (start == blockchain.end()) {
    lock.unlock();
    callback(std::error_code());
    return;
  }

  m_lastHeight = static_cast<uint32_t>(std::distance(blockchain.begin(), start));
  startHeight = m_lastHeight; 

  for (; m_lastHeight < blockchain.size(); ++m_lastHeight)
  {
    block_complete_entry_t e;
    e.block = IBinary::to(BinaryArray::to(blockchain[m_lastHeight]));

    for (auto hash : blockchain[m_lastHeight].transactionHashes)
    {
      transaction_t tx;
      if (!m_blockchainGenerator.getTransactionByHash(hash, tx))
        continue;

      e.txs.push_back(IBinary::to(BinaryArray::to(tx)));
    }

    newBlocks.push_back(e);

    if (newBlocks.size() >= m_getMaxBlocks) {
      break;
    }
  }

  m_lastHeight = startHeight + static_cast<uint32_t>(newBlocks.size());
  // m_lastHeight = startHeight + blockchain.size() - 1;

  lock.unlock();
  callback(std::error_code());
}

void INodeTrivialRefreshStub::getTransactionOutsGlobalIndices(const hash_t& transactionHash, std::vector<uint32_t>& outsGlobalIndices, const Callback& callback)
{
  m_asyncCounter.addAsyncContext();
  std::unique_lock<std::mutex> lock(m_walletLock);
  calls_getTransactionOutsGlobalIndices.push_back(transactionHash);
  std::thread task(&INodeTrivialRefreshStub::doGetTransactionOutsGlobalIndices, this, transactionHash, std::ref(outsGlobalIndices), callback);
  task.detach();
}

void INodeTrivialRefreshStub::doGetTransactionOutsGlobalIndices(const hash_t& transactionHash, std::vector<uint32_t>& outsGlobalIndices, const Callback& callback) {
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  bool success = m_blockchainGenerator.getTransactionGlobalIndexesByHash(transactionHash, outsGlobalIndices);

  lock.unlock();

  if (consumerTests) {
    outsGlobalIndices.clear();
    outsGlobalIndices.resize(20);
    getGlobalOutsFunctor(transactionHash, outsGlobalIndices);
    callback(std::error_code());
  } else {
    if (success) {
      callback(std::error_code());
    } else {
      callback(std::make_error_code(std::errc::invalid_argument));
    }
  }
}

void INodeTrivialRefreshStub::relayTransaction(const transaction_t& transaction, const Callback& callback)
{
  m_asyncCounter.addAsyncContext();
  std::thread task(&INodeTrivialRefreshStub::doRelayTransaction, this, transaction, callback);
  task.detach();
}

void INodeTrivialRefreshStub::doRelayTransaction(const transaction_t& transaction, const Callback& callback)
{
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  if (m_nextTxError)
  {
    m_nextTxError = false;
    lock.unlock();
    callback(make_error_code(error::INTERNAL_WALLET_ERROR));
    return;
  }

  if (m_nextTxToPool) {
    m_nextTxToPool = false;
    m_blockchainGenerator.putTxToPool(transaction);
    lock.unlock();
    callback(std::error_code());
    return;
  }

  m_blockchainGenerator.addTxToBlockchain(transaction);
  lock.unlock();
  callback(std::error_code());
}

void INodeTrivialRefreshStub::getRandomOutsByAmounts(std::vector<uint64_t>&& amounts, uint64_t outsCount, std::vector<COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount>& result, const Callback& callback)
{
  m_asyncCounter.addAsyncContext();
  std::thread task(&INodeTrivialRefreshStub::doGetRandomOutsByAmounts, this, amounts, outsCount, std::ref(result), callback);
  task.detach();
}

void INodeTrivialRefreshStub::doGetRandomOutsByAmounts(std::vector<uint64_t> amounts, uint64_t outsCount, std::vector<COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount>& result, const Callback& callback)
{
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  std::sort(amounts.begin(), amounts.end());

  for (uint64_t amount: amounts)
  {
    COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount out;
    out.amount = amount;

    uint64_t count = std::min(outsCount, m_maxMixin);

    for (uint32_t i = 0; i < count; ++i)
    {
      public_key_t key;
      secret_key_t sk;
      generate_keys((uint8_t*)&key, (uint8_t*)&sk);

      COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::out_entry e;
      e.global_amount_index = i;
      e.out_key = key;

      out.outs.push_back(e);
    }

    result.push_back(std::move(out));
  }

  lock.unlock();
  callback(std::error_code());
}

void INodeTrivialRefreshStub::queryBlocks(std::vector<hash_t>&& knownBlockIds, uint64_t timestamp,
        std::vector<BlockShortEntry>& newBlocks, uint32_t& startHeight, const Callback& callback) {
  auto resultHolder = std::make_shared<std::vector<block_complete_entry_t>>();

  getNewBlocks(std::move(knownBlockIds), *resultHolder, startHeight, [resultHolder, callback, &newBlocks](std::error_code ec)
  {
    if (ec) {
      callback(ec);
      return;
    }

    for (const auto& item : *resultHolder) {
      BlockShortEntry entry;

      if (!BinaryArray::from(entry.block, IBinary::from(item.block))) {
        callback(std::make_error_code(std::errc::invalid_argument));
        return;
      }

      entry.hasBlock = true;
      entry.blockHash = Block::getHash(entry.block);

      for (const auto& txBlob: item.txs) {
        transaction_t tx;
        if (!BinaryArray::from(tx, IBinary::from(txBlob))) {
          callback(std::make_error_code(std::errc::invalid_argument));
          return;
        }

        TransactionShortInfo tsi;
        tsi.txId = BinaryArray::objectHash(tx);
        tsi.txPrefix = tx;

        entry.txsShortInfo.push_back(std::move(tsi));
      }

      newBlocks.push_back(std::move(entry));
    }

    callback(ec);
  });

}

void INodeTrivialRefreshStub::startAlternativeChain(uint32_t height)
{
  m_blockchainGenerator.cutBlockchain(height);
  m_lastHeight = height;
}

void INodeTrivialRefreshStub::setNextTransactionError()
{
  m_nextTxError = true;
}

void INodeTrivialRefreshStub::setNextTransactionToPool() {
  m_nextTxToPool = true;
}

void INodeTrivialRefreshStub::cleanTransactionPool() {
  m_blockchainGenerator.clearTxPool();
}

void INodeTrivialRefreshStub::getPoolSymmetricDifference(std::vector<hash_t>&& known_pool_tx_ids, hash_t known_block_id, bool& is_bc_actual,
        std::vector<std::unique_ptr<ITransactionReader>>& new_txs, std::vector<hash_t>& deleted_tx_ids, const Callback& callback)
{
  m_asyncCounter.addAsyncContext();
  std::thread task(
    [this, known_pool_tx_ids, known_block_id, &is_bc_actual, &new_txs, &deleted_tx_ids, callback] () mutable {
      this->doGetPoolSymmetricDifference(std::move(known_pool_tx_ids), known_block_id, is_bc_actual, new_txs, deleted_tx_ids, callback);
    }
  );
  task.detach();
}

void INodeTrivialRefreshStub::doGetPoolSymmetricDifference(std::vector<hash_t>&& known_pool_tx_ids, hash_t known_block_id, bool& is_bc_actual,
        std::vector<std::unique_ptr<ITransactionReader>>& new_txs, std::vector<hash_t>& deleted_tx_ids, const Callback& callback)
{
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  std::vector<transaction_t> txs;
  m_blockchainGenerator.getPoolSymmetricDifference(std::move(known_pool_tx_ids), known_block_id, is_bc_actual, txs, deleted_tx_ids);
  lock.unlock();

  std::error_code ec;
  try {
    for (const auto& tx: txs) {
      new_txs.push_back(createTransactionPrefix(tx));
    }
  } catch (std::system_error& ex) {
    ec = ex.code();
  } catch (std::exception&) {
    ec = make_error_code(std::errc::invalid_argument);
  }

  callback(ec);
}

void INodeTrivialRefreshStub::includeTransactionsFromPoolToBlock() {
  m_blockchainGenerator.putTxPoolToBlockchain();
}

INodeTrivialRefreshStub::~INodeTrivialRefreshStub() {
  m_asyncCounter.waitAsyncContextsFinish();
}

void INodeTrivialRefreshStub::setMaxMixinCount(uint64_t maxMixin) {
  m_maxMixin = maxMixin;
}

void INodeTrivialRefreshStub::getBlocks(const std::vector<uint32_t>& blockHeights, std::vector<std::vector<block_details_t>>& blocks, const Callback& callback) {
  m_asyncCounter.addAsyncContext();

  std::thread task(
    std::bind(
      static_cast<
        void(INodeTrivialRefreshStub::*)(
        const std::vector<uint32_t>&,
          std::vector<std::vector<block_details_t>>&, 
          const Callback&
        )
      >(&INodeTrivialRefreshStub::doGetBlocks),
      this,
      std::cref(blockHeights),
      std::ref(blocks),
      callback
    )
  );
  task.detach();
}

void INodeTrivialRefreshStub::doGetBlocks(const std::vector<uint32_t>& blockHeights, std::vector<std::vector<block_details_t>>& blocks, const Callback& callback) {
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  for (const uint32_t& height : blockHeights) {
    if (m_blockchainGenerator.getBlockchain().size() <= height) {
      lock.unlock();
      callback(std::error_code(EDOM, std::generic_category()));
      return;
    }
    block_details_t b = block_details_t();
    b.height = height;
    b.isOrphaned = false;
    hash_t hash = Block::getHash(m_blockchainGenerator.getBlockchain()[height]);
    b.hash = hash;
    if (!m_blockchainGenerator.getGeneratedTransactionsNumber(height, b.alreadyGeneratedTransactions)) {
      callback(std::error_code(EDOM, std::generic_category()));
      return;
    }
    std::vector<block_details_t> v;
    v.push_back(b);
    blocks.push_back(v);
  }

  lock.unlock();
  callback(std::error_code());
}

void INodeTrivialRefreshStub::getBlocks(const std::vector<hash_t>& blockHashes, std::vector<block_details_t>& blocks, const Callback& callback) {
  m_asyncCounter.addAsyncContext();

  std::thread task(
    std::bind(
      static_cast<
        void(INodeTrivialRefreshStub::*)(
          const std::vector<hash_t>&, 
          std::vector<block_details_t>&, 
          const Callback&
        )
      >(&INodeTrivialRefreshStub::doGetBlocks),
      this,
      std::cref(blockHashes),
      std::ref(blocks),
      callback
    )
  );
  task.detach();
}

void INodeTrivialRefreshStub::doGetBlocks(const std::vector<hash_t>& blockHashes, std::vector<block_details_t>& blocks, const Callback& callback) {
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  for (const hash_t& hash: blockHashes) {
    auto iter = std::find_if(
        m_blockchainGenerator.getBlockchain().begin(), 
        m_blockchainGenerator.getBlockchain().end(), 
        [&hash](const block_t& block) -> bool {
          return hash == Block::getHash(block);
        }
    );
    if (iter == m_blockchainGenerator.getBlockchain().end()) {
      lock.unlock();
      callback(std::error_code(EDOM, std::generic_category()));
      return;
    }
    block_details_t b = block_details_t();
    hash_t actualHash = Block::getHash(*iter);
    b.hash = actualHash;
    b.isOrphaned = false;
    blocks.push_back(b);
  }

  lock.unlock();
  callback(std::error_code());
}

void INodeTrivialRefreshStub::getBlocks(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<block_details_t>& blocks, uint32_t& blocksNumberWithinTimestamps, const Callback& callback) {
  m_asyncCounter.addAsyncContext();

  std::thread task(
    std::bind(
      static_cast<
        void(INodeTrivialRefreshStub::*)(
          uint64_t, 
          uint64_t,
          uint32_t,
          std::vector<block_details_t>&, 
          uint32_t&,
          const Callback&
        )
      >(&INodeTrivialRefreshStub::doGetBlocks),
      this,
      timestampBegin,
      timestampEnd,
      blocksNumberLimit,
      std::ref(blocks),
      std::ref(blocksNumberWithinTimestamps),
      callback
    )
  );
  task.detach();
}

void INodeTrivialRefreshStub::doGetBlocks(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<block_details_t>& blocks, uint32_t& blocksNumberWithinTimestamps, const Callback& callback) {
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  std::vector<hash_t> blockHashes;
  if (!m_blockchainGenerator.getBlockIdsByTimestamp(timestampBegin, timestampEnd, blocksNumberLimit, blockHashes, blocksNumberWithinTimestamps)) {
    callback(std::error_code(EDOM, std::generic_category()));
    return;
  }

  for (const hash_t& hash: blockHashes) {
    auto iter = std::find_if(
        m_blockchainGenerator.getBlockchain().begin(), 
        m_blockchainGenerator.getBlockchain().end(), 
        [&hash](const block_t& block) -> bool {
          return hash == Block::getHash(block);
        }
    );
    if (iter == m_blockchainGenerator.getBlockchain().end()) {
      callback(std::error_code(EDOM, std::generic_category()));
      return;
    }
    block_details_t b = block_details_t();
    hash_t actualHash = Block::getHash(*iter);
    b.hash = actualHash;
    b.isOrphaned = false;
    b.timestamp = iter->timestamp;
    blocks.push_back(b);
  }

  callback(std::error_code());
}

void INodeTrivialRefreshStub::getTransactions(const std::vector<hash_t>& transactionHashes, std::vector<transaction_details_t>& transactions, const Callback& callback) {
  m_asyncCounter.addAsyncContext();

  std::thread task(
    std::bind(
      static_cast<
        void(INodeTrivialRefreshStub::*)(
          const std::vector<hash_t>&, 
          std::vector<transaction_details_t>&, 
          const Callback&
        )
      >(&INodeTrivialRefreshStub::doGetTransactions),
      this,
      std::cref(transactionHashes),
      std::ref(transactions),
      callback
    )
  );
  task.detach();
}

void INodeTrivialRefreshStub::doGetTransactions(const std::vector<hash_t>& transactionHashes, std::vector<transaction_details_t>& transactions, const Callback& callback) {
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  for (const hash_t& hash : transactionHashes) {
    transaction_t tx;
    transaction_details_t txDetails = transaction_details_t();
    if (m_blockchainGenerator.getTransactionByHash(hash, tx, false)) {
      hash_t actualHash = BinaryArray::objectHash(tx);
      txDetails.hash = actualHash;
      txDetails.inBlockchain = true;
    } else if (m_blockchainGenerator.getTransactionByHash(hash, tx, true)) {
      hash_t actualHash = BinaryArray::objectHash(tx);
      txDetails.hash = actualHash;
      txDetails.inBlockchain = false;
    } else {
      lock.unlock();
      callback(std::error_code(EDOM, std::generic_category()));
      return;
    }
    
    transactions.push_back(txDetails);
  }

  lock.unlock();
  callback(std::error_code());
}

void INodeTrivialRefreshStub::getPoolTransactions(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<transaction_details_t>& transactions, uint64_t& transactionsNumberWithinTimestamps, const Callback& callback) {
  m_asyncCounter.addAsyncContext();

  std::thread task(
    std::bind(
      &INodeTrivialRefreshStub::doGetPoolTransactions,
      this,
      timestampBegin,
      timestampEnd,
      transactionsNumberLimit,
      std::ref(transactions),
      std::ref(transactionsNumberWithinTimestamps),
      callback
    )
  );
  task.detach();
}

void INodeTrivialRefreshStub::doGetPoolTransactions(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<transaction_details_t>& transactions, uint64_t& transactionsNumberWithinTimestamps, const Callback& callback) {
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  std::vector<hash_t> transactionHashes;
  if (!m_blockchainGenerator.getPoolTransactionIdsByTimestamp(timestampBegin, timestampEnd, transactionsNumberLimit, transactionHashes, transactionsNumberWithinTimestamps)) {
    callback(std::error_code(EDOM, std::generic_category()));
    return;
  }

  for (const hash_t& hash : transactionHashes) {
    transaction_t tx;
    transaction_details_t txDetails = transaction_details_t();
    if (m_blockchainGenerator.getTransactionByHash(hash, tx, false)) {
      hash_t actualHash = BinaryArray::objectHash(tx);
      txDetails.hash = actualHash;
      txDetails.inBlockchain = true;
    } else if (m_blockchainGenerator.getTransactionByHash(hash, tx, true)) {
      hash_t actualHash = BinaryArray::objectHash(tx);
      txDetails.hash = actualHash;
      txDetails.inBlockchain = false;
    } else {
      callback(std::error_code(EDOM, std::generic_category()));
      return;
    }
    
    transactions.push_back(txDetails);
  }

  callback(std::error_code());
}

void INodeTrivialRefreshStub::getTransactionsByPaymentId(const hash_t& paymentId, std::vector<transaction_details_t>& transactions, const Callback& callback) {
  m_asyncCounter.addAsyncContext();

  std::thread task(
    std::bind(
      &INodeTrivialRefreshStub::doGetTransactionsByPaymentId,
      this,
      std::cref(paymentId),
      std::ref(transactions),
      callback
    )
  );
  task.detach();
}

void INodeTrivialRefreshStub::doGetTransactionsByPaymentId(const hash_t& paymentId, std::vector<transaction_details_t>& transactions, const Callback& callback) {
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  std::vector<hash_t> transactionHashes;
  if (!m_blockchainGenerator.getTransactionIdsByPaymentId(paymentId, transactionHashes)) {
    callback(std::error_code(EDOM, std::generic_category()));
    return;
  }

  for (const hash_t& hash : transactionHashes) {
    transaction_t tx;
    transaction_details_t txDetails = transaction_details_t();
    if (m_blockchainGenerator.getTransactionByHash(hash, tx, false)) {
      hash_t actualHash = BinaryArray::objectHash(tx);
      txDetails.hash = actualHash;
      txDetails.inBlockchain = true;
      hash_t paymentId;
      BlockchainExplorerDataBuilder::getPaymentId(tx, paymentId);
      txDetails.paymentId = paymentId;
    } else if (m_blockchainGenerator.getTransactionByHash(hash, tx, true)) {
      hash_t actualHash = BinaryArray::objectHash(tx);
      txDetails.hash =actualHash;
      txDetails.inBlockchain = false;
      hash_t paymentId;
      BlockchainExplorerDataBuilder::getPaymentId(tx, paymentId);
      txDetails.paymentId = paymentId;
    } else {
      callback(std::error_code(EDOM, std::generic_category()));
      return;
    }
    
    transactions.push_back(txDetails);
  }

  callback(std::error_code());
}

void INodeTrivialRefreshStub::isSynchronized(bool& syncStatus, const Callback& callback) {
  //m_asyncCounter.addAsyncContext();
  syncStatus = m_synchronized;
  callback(std::error_code());
}

void INodeTrivialRefreshStub::setSynchronizedStatus(bool status) {
  m_synchronized = status;
  if (m_synchronized) {
    observerManager.notify(&INodeObserver::blockchainSynchronized, getLastLocalBlockHeight());
  }
}

void INodeTrivialRefreshStub::sendPoolChanged() {
  observerManager.notify(&INodeObserver::poolChanged);
}

void INodeTrivialRefreshStub::sendLocalBlockchainUpdated(){
  observerManager.notify(&INodeObserver::localBlockchainUpdated, getLastLocalBlockHeight());
}

void INodeTrivialRefreshStub::getMultisignatureOutputByGlobalIndex(uint64_t amount, uint32_t gindex, cryptonote::multi_signature_output_t& out, const Callback& callback) {
  m_asyncCounter.addAsyncContext();
  std::unique_lock<std::mutex> lock(m_walletLock);
  std::thread task(&INodeTrivialRefreshStub::doGetOutByMSigGIndex, this, amount, gindex, std::ref(out), callback);
  task.detach();
}

void INodeTrivialRefreshStub::doGetOutByMSigGIndex(uint64_t amount, uint32_t gindex, cryptonote::multi_signature_output_t& out, const Callback& callback) {
  ContextCounterHolder counterHolder(m_asyncCounter);
  std::unique_lock<std::mutex> lock(m_walletLock);

  bool success = m_blockchainGenerator.getMultisignatureOutputByGlobalIndex(amount, gindex, out);

  lock.unlock();

  if (success) {
    callback(std::error_code());
  } else {
    callback(std::make_error_code(std::errc::invalid_argument));
  }
}
