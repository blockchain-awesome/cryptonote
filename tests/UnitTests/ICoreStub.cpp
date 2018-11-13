// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ICoreStub.h"

#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/IBlock.h"
#include "cryptonote/core/VerificationContext.h"
#include "cryptonote/structures/block_entry.h"

ICoreStub::ICoreStub() :
    topHeight(0),
    globalIndicesResult(false),
    randomOutsResult(false),
    poolTxVerificationResult(true),
    poolChangesResult(true) {
}

ICoreStub::ICoreStub(const cryptonote::block_t& genesisBlock) :
    topHeight(0),
    globalIndicesResult(false),
    randomOutsResult(false),
    poolTxVerificationResult(true),
    poolChangesResult(true) {
  addBlock(genesisBlock);
}

bool ICoreStub::addObserver(cryptonote::ICoreObserver* observer) {
  return true;
}

bool ICoreStub::removeObserver(cryptonote::ICoreObserver* observer) {
  return true;
}

void ICoreStub::get_blockchain_top(uint32_t& height, crypto::hash_t& top_id) {
  height = topHeight;
  top_id = topId;
}

std::vector<crypto::hash_t> ICoreStub::findBlockchainSupplement(const std::vector<crypto::hash_t>& remoteBlockIds, size_t maxCount,
  uint32_t& totalBlockCount, uint32_t& startBlockIndex) {

  //Sending all blockchain
  totalBlockCount = static_cast<uint32_t>(blocks.size());
  startBlockIndex = 0;
  std::vector<crypto::hash_t> result;
  result.reserve(std::min(blocks.size(), maxCount));
  for (uint32_t height = 0; height < static_cast<uint32_t>(std::min(blocks.size(), maxCount)); ++height) {
    assert(blockHashByHeightIndex.count(height) > 0);
    result.push_back(blockHashByHeightIndex[height]);
  }
  return result;
}

bool ICoreStub::get_random_outs_for_amounts(const cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_request& req,
    cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_response& res) {
  res = randomOuts;
  return randomOutsResult;
}

bool ICoreStub::get_tx_outputs_gindexs(const crypto::hash_t& tx_id, std::vector<uint32_t>& indexs) {
  std::copy(globalIndices.begin(), globalIndices.end(), std::back_inserter(indexs));
  return globalIndicesResult;
}

cryptonote::ICryptonoteProtocol* ICoreStub::get_protocol() {
  return nullptr;
}

bool ICoreStub::handle_incoming_tx(cryptonote::binary_array_t const& tx_blob, cryptonote::tx_verification_context_t& tvc, bool keeped_by_block) {
  return true;
}

void ICoreStub::set_blockchain_top(uint32_t height, const crypto::hash_t& top_id) {
  topHeight = height;
  topId = top_id;
}

void ICoreStub::set_outputs_gindexs(const std::vector<uint32_t>& indexs, bool result) {
  globalIndices.clear();
  std::copy(indexs.begin(), indexs.end(), std::back_inserter(globalIndices));
  globalIndicesResult = result;
}

void ICoreStub::set_random_outs(const cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_response& resp, bool result) {
  randomOuts = resp;
  randomOutsResult = result;
}

std::vector<cryptonote::transaction_t> ICoreStub::getPoolTransactions() {
  return std::vector<cryptonote::transaction_t>();
}

bool ICoreStub::getPoolChanges(const crypto::hash_t& tailBlockId, const std::vector<crypto::hash_t>& knownTxsIds,
                               std::vector<cryptonote::transaction_t>& addedTxs, std::vector<crypto::hash_t>& deletedTxsIds) {
  std::unordered_set<crypto::hash_t> knownSet;
  for (const crypto::hash_t& txId : knownTxsIds) {
    if (transactionPool.find(txId) == transactionPool.end()) {
      deletedTxsIds.push_back(txId);
    }

    knownSet.insert(txId);
  }

  for (const std::pair<crypto::hash_t, cryptonote::transaction_t>& poolEntry : transactionPool) {
    if (knownSet.find(poolEntry.first) == knownSet.end()) {
      addedTxs.push_back(poolEntry.second);
    }
  }

  return poolChangesResult;
}

bool ICoreStub::getPoolChangesLite(const crypto::hash_t& tailBlockId, const std::vector<crypto::hash_t>& knownTxsIds,
        std::vector<cryptonote::transaction_prefix_info_t>& addedTxs, std::vector<crypto::hash_t>& deletedTxsIds) {
  std::vector<cryptonote::transaction_t> added;
  bool returnStatus = getPoolChanges(tailBlockId, knownTxsIds, added, deletedTxsIds);

  for (const auto& tx : added) {
    cryptonote::transaction_prefix_info_t tpi;
    tpi.txPrefix = tx;
    tpi.txHash = getObjectHash(tx);

    addedTxs.push_back(std::move(tpi));
  }

  return returnStatus;
}

void ICoreStub::getPoolChanges(const std::vector<crypto::hash_t>& knownTxsIds, std::vector<cryptonote::transaction_t>& addedTxs,
                               std::vector<crypto::hash_t>& deletedTxsIds) {
}

bool ICoreStub::queryBlocks(const std::vector<crypto::hash_t>& block_ids, uint64_t timestamp,
  uint32_t& start_height, uint32_t& current_height, uint32_t& full_offset, std::vector<cryptonote::block_full_info_t>& entries) {
  //stub
  return true;
}

bool ICoreStub::queryBlocksLite(const std::vector<crypto::hash_t>& block_ids, uint64_t timestamp,
  uint32_t& start_height, uint32_t& current_height, uint32_t& full_offset, std::vector<cryptonote::block_short_info_t>& entries) {
  //stub
  return true;
}

std::vector<crypto::hash_t> ICoreStub::buildSparseChain() {
  std::vector<crypto::hash_t> result;
  result.reserve(blockHashByHeightIndex.size());
  for (auto kvPair : blockHashByHeightIndex) {
    result.emplace_back(kvPair.second);
  }

  std::reverse(result.begin(), result.end());
  return result;
}

std::vector<crypto::hash_t> ICoreStub::buildSparseChain(const crypto::hash_t& startBlockId) {
  // TODO implement
  assert(blocks.count(startBlockId) > 0);
  std::vector<crypto::hash_t> result;
  result.emplace_back(blockHashByHeightIndex[0]);
  return result;
}

size_t ICoreStub::addChain(const std::vector<const cryptonote::IBlock*>& chain) {
  size_t blocksCounter = 0;
  for (const cryptonote::IBlock* block : chain) {
    for (size_t txNumber = 0; txNumber < block->getTransactionCount(); ++txNumber) {
      const cryptonote::transaction_t& tx = block->getTransaction(txNumber);
      crypto::hash_t txHash = cryptonote::NULL_HASH;
      size_t blobSize = 0;
      getObjectHash(tx, txHash, blobSize);
      addTransaction(tx);
    }
    addBlock(block->getBlock());
    ++blocksCounter;
  }

  return blocksCounter;
}

crypto::hash_t ICoreStub::getBlockIdByHeight(uint32_t height) {
  auto iter = blockHashByHeightIndex.find(height);
  if (iter == blockHashByHeightIndex.end()) {
    return cryptonote::NULL_HASH;
  }
  return iter->second;
}

bool ICoreStub::getBlockByHash(const crypto::hash_t &h, cryptonote::block_t &blk) {
  auto iter = blocks.find(h);
  if (iter == blocks.end()) {
    return false;
  }
  blk = iter->second;
  return true;
}

void ICoreStub::getTransactions(const std::vector<crypto::hash_t>& txs_ids, std::list<cryptonote::transaction_t>& txs, std::list<crypto::hash_t>& missed_txs, bool checkTxPool) {
  for (const crypto::hash_t& hash : txs_ids) {
    auto iter = transactions.find(hash);
    if (iter != transactions.end()) {
      txs.push_back(iter->second);
    } else {
      missed_txs.push_back(hash);
    }
  }
  if (checkTxPool) {
    std::list<crypto::hash_t> pullTxIds(std::move(missed_txs));
    missed_txs.clear();
    for (const crypto::hash_t& hash : pullTxIds) {
      auto iter = transactionPool.find(hash);
      if (iter != transactionPool.end()) {
        txs.push_back(iter->second);
      }
      else {
        missed_txs.push_back(hash);
      }
    }
  }
}

bool ICoreStub::getBackwardBlocksSizes(uint32_t fromHeight, std::vector<size_t>& sizes, size_t count) {
  return true;
}

bool ICoreStub::getBlockSize(const crypto::hash_t& hash, size_t& size) {
  return true;
}

bool ICoreStub::getAlreadyGeneratedCoins(const crypto::hash_t& hash, uint64_t& generatedCoins) {
  return true;
}

bool ICoreStub::getBlockReward(size_t medianSize, size_t currentBlockSize, uint64_t alreadyGeneratedCoins, uint64_t fee,
    uint64_t& reward, int64_t& emissionChange) {
  return true;
}

bool ICoreStub::scanOutputkeysForIndices(const cryptonote::key_input_t& txInToKey, std::list<std::pair<crypto::hash_t, size_t>>& outputReferences) {
  return true;
}

bool ICoreStub::getBlockDifficulty(uint32_t height, cryptonote::difficulty_t& difficulty) {
  return true;
}

bool ICoreStub::getBlockContainingTx(const crypto::hash_t& txId, crypto::hash_t& blockId, uint32_t& blockHeight) {
  auto iter = blockHashByTxHashIndex.find(txId);
  if (iter == blockHashByTxHashIndex.end()) {
    return false;
  }
  blockId = iter->second;
  auto blockIter = blocks.find(blockId);
  if (blockIter == blocks.end()) {
    return false;
  }
  blockHeight = boost::get<cryptonote::base_input_t>(blockIter->second.baseTransaction.inputs.front()).blockIndex;
  return true;
}

bool ICoreStub::getMultisigOutputReference(const cryptonote::multi_signature_input_t& txInMultisig, std::pair<crypto::hash_t, size_t>& outputReference) {
  return true;
}

void ICoreStub::addBlock(const cryptonote::block_t& block) {
  uint32_t height = boost::get<cryptonote::base_input_t>(block.baseTransaction.inputs.front()).blockIndex;
  crypto::hash_t hash = cryptonote::Block::getHash(block);
  if (height > topHeight) {
    topHeight = height;
    topId = hash;
  }
  blocks.emplace(std::make_pair(hash, block));
  blockHashByHeightIndex.emplace(std::make_pair(height, hash));

  blockHashByTxHashIndex.emplace(std::make_pair(cryptonote::getObjectHash(block.baseTransaction), hash));
  for (auto txHash : block.transactionHashes) {
    blockHashByTxHashIndex.emplace(std::make_pair(txHash, hash));
  }
}

void ICoreStub::addTransaction(const cryptonote::transaction_t& tx) {
  crypto::hash_t hash = cryptonote::getObjectHash(tx);
  transactions.emplace(std::make_pair(hash, tx));
}

bool ICoreStub::getGeneratedTransactionsNumber(uint32_t height, uint64_t& generatedTransactions) {
  return true;
}

bool ICoreStub::getOrphanBlocksByHeight(uint32_t height, std::vector<cryptonote::block_t>& blocks) {
  return true;
}

bool ICoreStub::getBlocksByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<cryptonote::block_t>& blocks, uint32_t& blocksNumberWithinTimestamps) {
  return true;
}

bool ICoreStub::getPoolTransactionsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<cryptonote::transaction_t>& transactions, uint64_t& transactionsNumberWithinTimestamps) {
  return true;
}

bool ICoreStub::getTransactionsByPaymentId(const crypto::hash_t& paymentId, std::vector<cryptonote::transaction_t>& transactions) {
  return true;
}

std::error_code ICoreStub::executeLocked(const std::function<std::error_code()>& func) {
  return func();
}

std::unique_ptr<cryptonote::IBlock> ICoreStub::getBlock(const crypto::hash_t& blockId) {
  return std::unique_ptr<cryptonote::IBlock>(nullptr);
}

bool ICoreStub::handleIncomingTransaction(const cryptonote::transaction_t& tx, const crypto::hash_t& txHash, size_t blobSize, cryptonote::tx_verification_context_t& tvc, bool keptByBlock) {
  auto result = transactionPool.emplace(std::make_pair(txHash, tx));
  tvc.m_verifivation_failed = !poolTxVerificationResult;
  tvc.m_added_to_pool = true;
  tvc.m_should_be_relayed = result.second;
  return poolTxVerificationResult;
}

bool ICoreStub::have_block(const crypto::hash_t& id) {
  return blocks.count(id) > 0;
}

void ICoreStub::setPoolTxVerificationResult(bool result) {
  poolTxVerificationResult = result;
}

bool ICoreStub::addMessageQueue(cryptonote::MessageQueue<cryptonote::BlockchainMessage>& messageQueuePtr) {
  return true;
}

bool ICoreStub::removeMessageQueue(cryptonote::MessageQueue<cryptonote::BlockchainMessage>& messageQueuePtr) {
  return true;
}

void ICoreStub::setPoolChangesResult(bool result) {
  poolChangesResult = result;
}
