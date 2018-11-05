// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "InProcessNode.h"

#include <functional>
#include <boost/utility/value_init.hpp>
#include <cryptonote/core/TransactionApi.h>

#include "CryptoNoteConfig.h"
#include "common/StringTools.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/IBlock.h"
#include "cryptonote/core/VerificationContext.h"
#include "cryptonote/protocol/handler_common.h"
#include "InProcessNodeErrors.h"
#include "common/StringTools.h"

using namespace crypto;
using namespace Common;

namespace cryptonote {

InProcessNode::InProcessNode(cryptonote::ICore& core, cryptonote::ICryptoNoteProtocolQuery& protocol) :
    state(NOT_INITIALIZED),
    core(core),
    protocol(protocol),
    blockchainExplorerDataBuilder(core, protocol)
{
}

InProcessNode::~InProcessNode() {
  doShutdown();
}

bool InProcessNode::addObserver(INodeObserver* observer) {
  if (state != INITIALIZED) {
    throw std::system_error(make_error_code(cryptonote::error::NOT_INITIALIZED));
  }

  return observerManager.add(observer);
}

bool InProcessNode::removeObserver(INodeObserver* observer) {
  if (state != INITIALIZED) {
    throw std::system_error(make_error_code(cryptonote::error::NOT_INITIALIZED));
  }

  return observerManager.remove(observer);
}

void InProcessNode::init(const Callback& callback) {
  std::unique_lock<std::mutex> lock(mutex);
  std::error_code ec;

  if (state != NOT_INITIALIZED) {
    ec = make_error_code(cryptonote::error::ALREADY_INITIALIZED);
  } else {
    protocol.addObserver(this);
    core.addObserver(this);

    work.reset(new boost::asio::io_service::work(ioService));
    workerThread.reset(new std::thread(&InProcessNode::workerFunc, this));

    state = INITIALIZED;
  }

  ioService.post(std::bind(callback, ec));
}

bool InProcessNode::shutdown() {
  return doShutdown();
}

bool InProcessNode::doShutdown() {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    return false;
  }

  protocol.removeObserver(this);
  core.removeObserver(this);
  state = NOT_INITIALIZED;

  work.reset();
  ioService.stop();
  workerThread->join();
  ioService.reset();
  return true;
}

void InProcessNode::workerFunc() {
  ioService.run();
}

void InProcessNode::getNewBlocks(std::vector<crypto::hash_t>&& knownBlockIds, std::vector<cryptonote::block_complete_entry>& newBlocks,
  uint32_t& startHeight, const Callback& callback)
{
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
    std::bind(&InProcessNode::getNewBlocksAsync,
      this,
      std::move(knownBlockIds),
      std::ref(newBlocks),
      std::ref(startHeight),
      callback
    )
  );
}

void InProcessNode::getNewBlocksAsync(std::vector<crypto::hash_t>& knownBlockIds, std::vector<cryptonote::block_complete_entry>& newBlocks,
  uint32_t& startHeight, const Callback& callback)
{
  std::error_code ec = doGetNewBlocks(std::move(knownBlockIds), newBlocks, startHeight);
  callback(ec);
}

//it's always protected with mutex
std::error_code InProcessNode::doGetNewBlocks(std::vector<crypto::hash_t>&& knownBlockIds, std::vector<cryptonote::block_complete_entry>& newBlocks, uint32_t& startHeight) {
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (state != INITIALIZED) {
      return make_error_code(cryptonote::error::NOT_INITIALIZED);
    }
  }

  try {
    // TODO code duplication see RpcServer::on_get_blocks()
    if (knownBlockIds.empty()) {
      return make_error_code(cryptonote::error::REQUEST_ERROR);
    }

    if (knownBlockIds.back() != core.getBlockIdByHeight(0)) {
      return make_error_code(cryptonote::error::REQUEST_ERROR);
    }

    uint32_t totalBlockCount;
    std::vector<crypto::hash_t> supplement = core.findBlockchainSupplement(knownBlockIds, cryptonote::COMMAND_RPC_GET_BLOCKS_FAST_MAX_COUNT, totalBlockCount, startHeight);

    for (const auto& blockId : supplement) {
      assert(core.have_block(blockId));
      auto completeBlock = core.getBlock(blockId);
      assert(completeBlock != nullptr);

      cryptonote::block_complete_entry be;
      be.block = asString(toBinaryArray(completeBlock->getBlock()));

      be.txs.reserve(completeBlock->getTransactionCount());
      for (size_t i = 0; i < completeBlock->getTransactionCount(); ++i) {
        be.txs.push_back(asString(toBinaryArray(completeBlock->getTransaction(i))));
      }

      newBlocks.push_back(std::move(be));
    }
  } catch (std::system_error& e) {
    return e.code();
  } catch (std::exception&) {
    return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
  }

  return std::error_code();
}

void InProcessNode::getTransactionOutsGlobalIndices(const crypto::hash_t& transactionHash, std::vector<uint32_t>& outsGlobalIndices,
    const Callback& callback)
{
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
    std::bind(&InProcessNode::getTransactionOutsGlobalIndicesAsync,
      this,
      std::cref(transactionHash),
      std::ref(outsGlobalIndices),
      callback
    )
  );
}

void InProcessNode::getTransactionOutsGlobalIndicesAsync(const crypto::hash_t& transactionHash, std::vector<uint32_t>& outsGlobalIndices,
    const Callback& callback)
{
  std::error_code ec = doGetTransactionOutsGlobalIndices(transactionHash, outsGlobalIndices);
  callback(ec);
}

//it's always protected with mutex
std::error_code InProcessNode::doGetTransactionOutsGlobalIndices(const crypto::hash_t& transactionHash, std::vector<uint32_t>& outsGlobalIndices) {
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (state != INITIALIZED) {
      return make_error_code(cryptonote::error::NOT_INITIALIZED);
    }
  }

  try {
    bool r = core.get_tx_outputs_gindexs(transactionHash, outsGlobalIndices);
    if(!r) {
      return make_error_code(cryptonote::error::REQUEST_ERROR);
    }
  } catch (std::system_error& e) {
    return e.code();
  } catch (std::exception&) {
    return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
  }

  return std::error_code();
}

void InProcessNode::getRandomOutsByAmounts(std::vector<uint64_t>&& amounts, uint64_t outsCount,
    std::vector<cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount>& result, const Callback& callback)
{
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
    std::bind(&InProcessNode::getRandomOutsByAmountsAsync,
      this,
      std::move(amounts),
      outsCount,
      std::ref(result),
      callback
    )
  );
}

void InProcessNode::getRandomOutsByAmountsAsync(std::vector<uint64_t>& amounts, uint64_t outsCount,
  std::vector<cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount>& result, const Callback& callback)
{
  std::error_code ec = doGetRandomOutsByAmounts(std::move(amounts), outsCount, result);
  callback(ec);
}

//it's always protected with mutex
std::error_code InProcessNode::doGetRandomOutsByAmounts(std::vector<uint64_t>&& amounts, uint64_t outsCount, std::vector<cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount>& result) {
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (state != INITIALIZED) {
      return make_error_code(cryptonote::error::NOT_INITIALIZED);
    }
  }

  try {
    cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::response res;
    cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::request req;
    req.amounts = amounts;
    req.outs_count = outsCount;

    if(!core.get_random_outs_for_amounts(req, res)) {
      return make_error_code(cryptonote::error::REQUEST_ERROR);
    }

    result = std::move(res.outs);
  } catch (std::system_error& e) {
    return e.code();
  } catch (std::exception&) {
    return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
  }

  return std::error_code();
}


void InProcessNode::relayTransaction(const cryptonote::transaction_t& transaction, const Callback& callback)
{
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
    std::bind(&InProcessNode::relayTransactionAsync,
      this,
      transaction,
      callback
    )
  );
}

void InProcessNode::relayTransactionAsync(const cryptonote::transaction_t& transaction, const Callback& callback) {
  std::error_code ec = doRelayTransaction(transaction);
  callback(ec);
}

//it's always protected with mutex
std::error_code InProcessNode::doRelayTransaction(const cryptonote::transaction_t& transaction) {
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (state != INITIALIZED) {
      return make_error_code(cryptonote::error::NOT_INITIALIZED);
    }
  }

  try {
    cryptonote::BinaryArray transactionBinaryArray = toBinaryArray(transaction);
    cryptonote::TxVerificationContext tvc = boost::value_initialized<cryptonote::TxVerificationContext>();

    if (!core.handle_incoming_tx(transactionBinaryArray, tvc, false)) {
      return make_error_code(cryptonote::error::REQUEST_ERROR);
    }

    if(tvc.m_verifivation_failed) {
      return make_error_code(cryptonote::error::REQUEST_ERROR);
    }

    if(!tvc.m_should_be_relayed) {
      return make_error_code(cryptonote::error::REQUEST_ERROR);
    }

    cryptonote::NOTIFY_NEW_TRANSACTIONS::request r;
    r.txs.push_back(asString(transactionBinaryArray));
    core.get_protocol()->relay_transactions(r);
  } catch (std::system_error& e) {
    return e.code();
  } catch (std::exception&) {
    return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
  }

  return std::error_code();
}

size_t InProcessNode::getPeerCount() const {
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (state != INITIALIZED) {
      throw std::system_error(make_error_code(cryptonote::error::NOT_INITIALIZED));
    }
  }

  return protocol.getPeerCount();
}

uint32_t InProcessNode::getLocalBlockCount() const {
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (state != INITIALIZED) {
      throw std::system_error(make_error_code(cryptonote::error::NOT_INITIALIZED));
    }
  }

  uint32_t lastIndex;
  crypto::hash_t ignore;

  core.get_blockchain_top(lastIndex, ignore);

  return lastIndex + 1;
}

uint32_t InProcessNode::getKnownBlockCount() const {
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (state != INITIALIZED) {
      throw std::system_error(make_error_code(cryptonote::error::NOT_INITIALIZED));
    }
  }

  return protocol.getObservedHeight();
}

uint32_t InProcessNode::getLastLocalBlockHeight() const {
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (state != INITIALIZED) {
      throw std::system_error(make_error_code(cryptonote::error::NOT_INITIALIZED));
    }
  }

  uint32_t height;
  crypto::hash_t ignore;

  core.get_blockchain_top(height, ignore);

  return height;
}

uint32_t InProcessNode::getLastKnownBlockHeight() const {
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (state != INITIALIZED) {
      throw std::system_error(make_error_code(cryptonote::error::NOT_INITIALIZED));
    }
  }

  return protocol.getObservedHeight() - 1;
}

uint64_t InProcessNode::getLastLocalBlockTimestamp() const {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    throw std::system_error(make_error_code(cryptonote::error::NOT_INITIALIZED));
  }
  lock.unlock();

  uint32_t ignore;
  crypto::hash_t hash;

  core.get_blockchain_top(ignore, hash);

  cryptonote::block_t block;
  if (!core.getBlockByHash(hash, block)) {
    throw std::system_error(make_error_code(cryptonote::error::INTERNAL_NODE_ERROR));
  }

  return block.timestamp;
}

void InProcessNode::peerCountUpdated(size_t count) {
  observerManager.notify(&INodeObserver::peerCountUpdated, count);
}

void InProcessNode::lastKnownBlockHeightUpdated(uint32_t height) {
  observerManager.notify(&INodeObserver::lastKnownBlockHeightUpdated, height);
}

void InProcessNode::blockchainUpdated() {
  uint32_t height;
  crypto::hash_t ignore;

  core.get_blockchain_top(height, ignore);
  observerManager.notify(&INodeObserver::localBlockchainUpdated, height);
}

void InProcessNode::poolUpdated() {
  observerManager.notify(&INodeObserver::poolChanged);
}

void InProcessNode::blockchainSynchronized(uint32_t topHeight) {
  observerManager.notify(&INodeObserver::blockchainSynchronized, topHeight);
}

void InProcessNode::queryBlocks(std::vector<crypto::hash_t>&& knownBlockIds, uint64_t timestamp, std::vector<BlockShortEntry>& newBlocks,
  uint32_t& startHeight, const Callback& callback) {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
          std::bind(&InProcessNode::queryBlocksLiteAsync,
                  this,
                  std::move(knownBlockIds),
                  timestamp,
                  std::ref(newBlocks),
                  std::ref(startHeight),
                  callback
          )
  );
}

void InProcessNode::queryBlocksLiteAsync(std::vector<crypto::hash_t>& knownBlockIds, uint64_t timestamp, std::vector<BlockShortEntry>& newBlocks, uint32_t& startHeight,
                         const Callback& callback) {
  std::error_code ec = doQueryBlocksLite(std::move(knownBlockIds), timestamp, newBlocks, startHeight);
  callback(ec);
}

std::error_code InProcessNode::doQueryBlocksLite(std::vector<crypto::hash_t>&& knownBlockIds, uint64_t timestamp, std::vector<BlockShortEntry>& newBlocks, uint32_t& startHeight) {
  uint32_t currentHeight, fullOffset;
  std::vector<cryptonote::block_short_info_t> entries;

  if (!core.queryBlocksLite(knownBlockIds, timestamp, startHeight, currentHeight, fullOffset, entries)) {
    return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
  }

  for (const auto& entry: entries) {
    BlockShortEntry bse;
    bse.blockHash = entry.blockId;
    bse.hasBlock = false;

    if (!entry.block.empty()) {
      bse.hasBlock = true;
      if (!fromBinaryArray(bse.block, asBinaryArray(entry.block))) {
        return std::make_error_code(std::errc::invalid_argument);
      }
    }

    for (const auto& tsi: entry.txPrefixes) {
      TransactionShortInfo tpi;
      tpi.txId = tsi.txHash;
      tpi.txPrefix = tsi.txPrefix;

      bse.txsShortInfo.push_back(std::move(tpi));
    }

    newBlocks.push_back(std::move(bse));
  }

  return std::error_code();

}

void InProcessNode::getPoolSymmetricDifference(std::vector<crypto::hash_t>&& knownPoolTxIds, crypto::hash_t knownBlockId, bool& isBcActual,
        std::vector<std::unique_ptr<ITransactionReader>>& newTxs, std::vector<crypto::hash_t>& deletedTxIds, const Callback& callback) {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post([this, knownPoolTxIds, knownBlockId, &isBcActual, &newTxs, &deletedTxIds, callback] () mutable {
    this->getPoolSymmetricDifferenceAsync(std::move(knownPoolTxIds), knownBlockId, isBcActual, newTxs, deletedTxIds, callback);
  });
}

void InProcessNode::getPoolSymmetricDifferenceAsync(std::vector<crypto::hash_t>&& knownPoolTxIds, crypto::hash_t knownBlockId, bool& isBcActual,
        std::vector<std::unique_ptr<ITransactionReader>>& newTxs, std::vector<crypto::hash_t>& deletedTxIds, const Callback& callback) {
  std::error_code ec = std::error_code();

  std::vector<transaction_prefix_info_t> added;
  isBcActual = core.getPoolChangesLite(knownBlockId, knownPoolTxIds, added, deletedTxIds);

  try {
    for (const auto& tx: added) {
      newTxs.push_back(createTransactionPrefix(tx.txPrefix, tx.txHash));
    }
  } catch (std::system_error& ex) {
    ec = ex.code();
  } catch (std::exception&) {
    ec = make_error_code(std::errc::invalid_argument);
  }

  callback(ec);
}

void InProcessNode::getMultisignatureOutputByGlobalIndex(uint64_t amount, uint32_t gindex, multi_signature_output_t& out, const Callback& callback) {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post([this, amount, gindex, &out, callback]() mutable {
    this->getOutByMSigGIndexAsync(amount, gindex, out, callback);
  });
}

void InProcessNode::getOutByMSigGIndexAsync(uint64_t amount, uint32_t gindex, multi_signature_output_t& out, const Callback& callback) {
  std::error_code ec = std::error_code();
  bool result = core.getOutByMSigGIndex(amount, gindex, out);
  if (!result) {
    ec = make_error_code(std::errc::invalid_argument);
    callback(ec);
    return;
  }

  callback(ec);
}

void InProcessNode::getBlocks(const std::vector<uint32_t>& blockHeights, std::vector<std::vector<BlockDetails>>& blocks, const Callback& callback) {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
    std::bind(
      static_cast<
        void(InProcessNode::*)(
        const std::vector<uint32_t>&,
          std::vector<std::vector<BlockDetails>>&, 
          const Callback&
        )
      >(&InProcessNode::getBlocksAsync),
      this,
      std::cref(blockHeights),
      std::ref(blocks),
      callback
    )
  );
}

void InProcessNode::getBlocksAsync(const std::vector<uint32_t>& blockHeights, std::vector<std::vector<BlockDetails>>& blocks, const Callback& callback) {
  std::error_code ec = core.executeLocked(
    std::bind(
      static_cast<
        std::error_code(InProcessNode::*)(
        const std::vector<uint32_t>&,
          std::vector<std::vector<BlockDetails>>&
        )
      >(&InProcessNode::doGetBlocks),
      this,
      std::cref(blockHeights),
      std::ref(blocks)
    )
  );
  callback(ec);
}

std::error_code InProcessNode::doGetBlocks(const std::vector<uint32_t>& blockHeights, std::vector<std::vector<BlockDetails>>& blocks) {
  try {
    uint32_t topHeight = 0;
    crypto::hash_t topHash = boost::value_initialized<crypto::hash_t>();
    core.get_blockchain_top(topHeight, topHash);
    for (const uint32_t& height : blockHeights) {
      if (height > topHeight) {
        return make_error_code(cryptonote::error::REQUEST_ERROR);
      }
      crypto::hash_t hash = core.getBlockIdByHeight(height);
      block_t block;
      if (!core.getBlockByHash(hash, block)) {
        return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
      }
      BlockDetails blockDetails;
      if (!blockchainExplorerDataBuilder.fillBlockDetails(block, blockDetails)) {
        return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
      }
      std::vector<BlockDetails> blocksOnSameHeight;
      blocksOnSameHeight.push_back(std::move(blockDetails));

      //Getting orphans
      std::vector<block_t> orphanBlocks;
      core.getOrphanBlocksByHeight(height, orphanBlocks);
      for (const block_t& orphanBlock : orphanBlocks) {
        BlockDetails orphanBlockDetails;
        if (!blockchainExplorerDataBuilder.fillBlockDetails(orphanBlock, orphanBlockDetails)) {
          return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
        }
        blocksOnSameHeight.push_back(std::move(orphanBlockDetails));
      }
      blocks.push_back(std::move(blocksOnSameHeight));
    }
  } catch (std::system_error& e) {
    return e.code();
  } catch (std::exception&) {
    return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
  }

  return std::error_code();
}

void InProcessNode::getBlocks(const std::vector<crypto::hash_t>& blockHashes, std::vector<BlockDetails>& blocks, const Callback& callback) {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
    std::bind(
      static_cast<
        void(InProcessNode::*)(
          const std::vector<crypto::hash_t>&, 
          std::vector<BlockDetails>&, 
          const Callback&
        )
      >(&InProcessNode::getBlocksAsync),
      this,
      std::cref(blockHashes),
      std::ref(blocks),
      callback
    )
  );
}

void InProcessNode::getBlocksAsync(const std::vector<crypto::hash_t>& blockHashes, std::vector<BlockDetails>& blocks, const Callback& callback) {
  std::error_code ec = core.executeLocked(
    std::bind(
      static_cast<
        std::error_code(InProcessNode::*)(
          const std::vector<crypto::hash_t>&, 
          std::vector<BlockDetails>&
        )
      >(&InProcessNode::doGetBlocks),
      this,
      std::cref(blockHashes),
      std::ref(blocks)
    )
  );
  callback(ec);
}

std::error_code InProcessNode::doGetBlocks(const std::vector<crypto::hash_t>& blockHashes, std::vector<BlockDetails>& blocks) {
  try {
    for (const crypto::hash_t& hash : blockHashes) {
      block_t block;
      if (!core.getBlockByHash(hash, block)) {
        return make_error_code(cryptonote::error::REQUEST_ERROR);
      }
      BlockDetails blockDetails;
      if (!blockchainExplorerDataBuilder.fillBlockDetails(block, blockDetails)) {
        return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
      }
      blocks.push_back(std::move(blockDetails));
    }
  } catch (std::system_error& e) {
    return e.code();
  } catch (std::exception&) {
    return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
  }
  return std::error_code();
}

void InProcessNode::getBlocks(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<BlockDetails>& blocks, uint32_t& blocksNumberWithinTimestamps, const Callback& callback) {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
    std::bind(
      static_cast<
        void(InProcessNode::*)(
          uint64_t, 
          uint64_t, 
          uint32_t,
          std::vector<BlockDetails>&, 
          uint32_t&,
          const Callback&
        )
      >(&InProcessNode::getBlocksAsync),
      this,
      timestampBegin,
      timestampEnd,
      blocksNumberLimit,
      std::ref(blocks),
      std::ref(blocksNumberWithinTimestamps),
      callback
    )
  );
}

void InProcessNode::getBlocksAsync(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<BlockDetails>& blocks, uint32_t& blocksNumberWithinTimestamps, const Callback& callback) {
  std::error_code ec = core.executeLocked(
    std::bind(
      static_cast<
        std::error_code(InProcessNode::*)(
          uint64_t, 
          uint64_t, 
          uint32_t,
          std::vector<BlockDetails>&,
          uint32_t&
        )
      >(&InProcessNode::doGetBlocks),
      this,
      timestampBegin,
      timestampEnd,
      blocksNumberLimit,
      std::ref(blocks),
      std::ref(blocksNumberWithinTimestamps)
    )
  );

  callback(ec);
}

std::error_code InProcessNode::doGetBlocks(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<BlockDetails>& blocks, uint32_t& blocksNumberWithinTimestamps) {
  try {
    std::vector<block_t> rawBlocks;
    if (!core.getBlocksByTimestamp(timestampBegin, timestampEnd, blocksNumberLimit, rawBlocks, blocksNumberWithinTimestamps)) {
      return make_error_code(cryptonote::error::REQUEST_ERROR);
    }
    for (const block_t& rawBlock : rawBlocks) {
      BlockDetails block;
      if (!blockchainExplorerDataBuilder.fillBlockDetails(rawBlock, block)) {
        return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
      }
      blocks.push_back(std::move(block));
    }
  } catch (std::system_error& e) {
    return e.code();
  } catch (std::exception&) {
    return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
  }
  return std::error_code();
}

void InProcessNode::getTransactions(const std::vector<crypto::hash_t>& transactionHashes, std::vector<transaction_details_t>& transactions, const Callback& callback) {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
    std::bind(
      static_cast<
        void(InProcessNode::*)(
          const std::vector<crypto::hash_t>&, 
          std::vector<transaction_details_t>&, 
          const Callback&
        )
      >(&InProcessNode::getTransactionsAsync),
      this,
      std::cref(transactionHashes),
      std::ref(transactions),
      callback
    )
  );
}

void InProcessNode::getTransactionsAsync(const std::vector<crypto::hash_t>& transactionHashes, std::vector<transaction_details_t>& transactions, const Callback& callback) {
  std::error_code ec = core.executeLocked(
    std::bind(
      static_cast<
        std::error_code(InProcessNode::*)(
          const std::vector<crypto::hash_t>&, 
          std::vector<transaction_details_t>&
        )
      >(&InProcessNode::doGetTransactions),
      this,
      std::cref(transactionHashes),
      std::ref(transactions)
    )
  );
  callback(ec);
}

std::error_code InProcessNode::doGetTransactions(const std::vector<crypto::hash_t>& transactionHashes, std::vector<transaction_details_t>& transactions) {
  try {
    std::list<transaction_t> txs;
    std::list<crypto::hash_t> missed_txs;
    core.getTransactions(transactionHashes, txs, missed_txs, true);
    if (missed_txs.size() > 0) {
      return make_error_code(cryptonote::error::REQUEST_ERROR);
    }
    for (const transaction_t& tx : txs) {
      transaction_details_t transactionDetails;
      if (!blockchainExplorerDataBuilder.fillTransactionDetails(tx, transactionDetails)) {
        return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
      }
      transactions.push_back(std::move(transactionDetails));
    }
  } catch (std::system_error& e) {
    return e.code();
  } catch (std::exception&) {
    return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
  }
  return std::error_code();
}

void InProcessNode::getPoolTransactions(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<transaction_details_t>& transactions, uint64_t& transactionsNumberWithinTimestamps, const Callback& callback) {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
    std::bind(
      &InProcessNode::getPoolTransactionsAsync,
      this,
      timestampBegin,
      timestampEnd,
      transactionsNumberLimit,
      std::ref(transactions),
      std::ref(transactionsNumberWithinTimestamps),
      callback
    )
  );
}

void InProcessNode::getPoolTransactionsAsync(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<transaction_details_t>& transactions, uint64_t& transactionsNumberWithinTimestamps, const Callback& callback) {
  std::error_code ec = core.executeLocked(
    std::bind(
      &InProcessNode::doGetPoolTransactions,
      this,
      timestampBegin,
      timestampEnd,
      transactionsNumberLimit,
      std::ref(transactions),
      std::ref(transactionsNumberWithinTimestamps)
    )
  );

  callback(ec);
}

std::error_code InProcessNode::doGetPoolTransactions(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<transaction_details_t>& transactions, uint64_t& transactionsNumberWithinTimestamps) {
  try {
    std::vector<transaction_t> rawTransactions;
    if (!core.getPoolTransactionsByTimestamp(timestampBegin, timestampEnd, transactionsNumberLimit, rawTransactions, transactionsNumberWithinTimestamps)) {
      return make_error_code(cryptonote::error::REQUEST_ERROR);
    }
    for (const transaction_t& rawTransaction : rawTransactions) {
      transaction_details_t transactionDetails;
      if (!blockchainExplorerDataBuilder.fillTransactionDetails(rawTransaction, transactionDetails)) {
        return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
      }
      transactions.push_back(std::move(transactionDetails));
    }
  } catch (std::system_error& e) {
    return e.code();
  } catch (std::exception&) {
    return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
  }
  return std::error_code();
}

void InProcessNode::getTransactionsByPaymentId(const crypto::hash_t& paymentId, std::vector<transaction_details_t>& transactions, const Callback& callback) {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
    std::bind(
      &InProcessNode::getTransactionsByPaymentIdAsync,
      this,
      std::cref(paymentId),
      std::ref(transactions),
      callback
    )
  );
}

void InProcessNode::getTransactionsByPaymentIdAsync(const crypto::hash_t& paymentId, std::vector<transaction_details_t>& transactions, const Callback& callback) {
  std::error_code ec = core.executeLocked(
    std::bind(
      &InProcessNode::doGetTransactionsByPaymentId,
      this,
      paymentId,
      std::ref(transactions)
    )
  );

  callback(ec);
}

std::error_code InProcessNode::doGetTransactionsByPaymentId(const crypto::hash_t& paymentId, std::vector<transaction_details_t>& transactions) {
  try {
    std::vector<transaction_t> rawTransactions;
    if (!core.getTransactionsByPaymentId(paymentId, rawTransactions)) {
      return make_error_code(cryptonote::error::REQUEST_ERROR);
    }
    for (const transaction_t& rawTransaction : rawTransactions) {
      transaction_details_t transactionDetails;
      if (!blockchainExplorerDataBuilder.fillTransactionDetails(rawTransaction, transactionDetails)) {
        return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
      }
      transactions.push_back(std::move(transactionDetails));
    }
  } catch (std::system_error& e) {
    return e.code();
  } catch (std::exception&) {
    return make_error_code(cryptonote::error::INTERNAL_NODE_ERROR);
  }
  return std::error_code();
}

void InProcessNode::isSynchronized(bool& syncStatus, const Callback& callback) {
  std::unique_lock<std::mutex> lock(mutex);
  if (state != INITIALIZED) {
    lock.unlock();
    callback(make_error_code(cryptonote::error::NOT_INITIALIZED));
    return;
  }

  ioService.post(
    std::bind(
      &InProcessNode::isSynchronizedAsync,
      this,
      std::ref(syncStatus),
      callback
    )
  );
}

void InProcessNode::isSynchronizedAsync(bool& syncStatus, const Callback& callback) {
  syncStatus = protocol.isSynchronized();
  callback(std::error_code());
}

} //namespace cryptonote
