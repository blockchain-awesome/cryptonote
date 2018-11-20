// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blockchain.h"
#include <boost/filesystem.hpp>

#include <algorithm>
#include <cstdio>
#include <boost/foreach.hpp>
#include "common/Math.h"
#include "common/ShuffleGenerator.h"
#include "stream/StdInputStream.h"
#include "stream/StdOutputStream.h"
#include "rpc/CoreRpcServerCommandsDefinitions.h"
#include "serialization/BinarySerializationTools.h"
#include "CryptoNoteTools.h"

#include <cryptonote/core/blockchain/defines.h>
#include "cryptonote/core/blockchain/serializer/block_cache.hpp"
#include "cryptonote/core/blockchain/serializer/blockchain_indices.hpp"

using namespace Logging;
using namespace Common;

namespace std {
  template <typename T> bool operator < (const T& t1, const T& t2) {
    return memcmp(&t1, &t2, sizeof(T)) < 0;
  }
}

namespace cryptonote {

template<typename K, typename V, typename hash_t>
bool serialize(google::sparse_hash_map<K, V, hash_t>& value, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializeMap(value, name, serializer, [&value](size_t size) { value.resize(size); });
}

template<typename K, typename hash_t>
bool serialize(google::sparse_hash_set<K, hash_t>& value, Common::StringView name, cryptonote::ISerializer& serializer) {
  size_t size = value.size();
  if (!serializer.beginArray(size, name)) {
    return false;
  }

  if (serializer.type() == ISerializer::OUTPUT) {
    for (auto& key : value) {
      serializer(const_cast<K&>(key), "");
    }
  } else {
    value.resize(size);
    while (size--) {
      K key;
      serializer(key, "");
      value.insert(key);
    }
  }

  serializer.endArray();
  return true;
}

Blockchain::Blockchain(const Currency& currency, TxMemoryPool& tx_pool, ILogger& logger) :
logger(logger, "Blockchain"),
m_currency(currency),
m_tx_pool(tx_pool),
m_current_block_cumul_sz_limit(0),
m_is_in_checkpoint_zone(false),
m_blocks(currency),
m_checkpoints(logger) {

  m_outputs.set_deleted_key(0);
  crypto::key_image_t nullImage = boost::value_initialized<decltype(nullImage)>();
  m_spent_keys.set_deleted_key(nullImage);
}

bool Blockchain::addObserver(IBlockchainStorageObserver* observer) {
  return m_observerManager.add(observer);
}

bool Blockchain::removeObserver(IBlockchainStorageObserver* observer) {
  return m_observerManager.remove(observer);
}

bool Blockchain::checkTransactionInputs(const cryptonote::transaction_t& tx, block_info_t& maxUsedBlock) {
  return checkTransactionInputs(tx, maxUsedBlock.height, maxUsedBlock.id);
}

bool Blockchain::checkTransactionInputs(const cryptonote::transaction_t& tx, block_info_t& maxUsedBlock, block_info_t& lastFailed) {

  block_info_t tail;

  //not the best implementation at this time, sorry :(
  //check is ring_signature already checked ?
  if (maxUsedBlock.empty()) {
    //not checked, lets try to check
    if (!lastFailed.empty() && getHeight() > lastFailed.height && getBlockIdByHeight(lastFailed.height) == lastFailed.id) {
      return false; //we already sure that this tx is broken for this height
    }

    if (!checkTransactionInputs(tx, maxUsedBlock.height, maxUsedBlock.id, &tail)) {
      lastFailed = tail;
      return false;
    }
  } else {
    if (maxUsedBlock.height >= getHeight()) {
      return false;
    }

    if (getBlockIdByHeight(maxUsedBlock.height) != maxUsedBlock.id) {
      //if we already failed on this height and id, skip actual ring signature check
      if (lastFailed.id == getBlockIdByHeight(lastFailed.height)) {
        return false;
      }

      //check ring signature again, it is possible (with very small chance) that this transaction become again valid
      if (!checkTransactionInputs(tx, maxUsedBlock.height, maxUsedBlock.id, &tail)) {
        lastFailed = tail;
        return false;
      }
    }
  }

  return true;
}

bool Blockchain::haveSpentKeyImages(const cryptonote::transaction_t& tx) {
  return this->haveTransactionKeyImagesAsSpent(tx);
}

/**
* \pre m_blockchain_lock is locked
*/
bool Blockchain::checkTransactionSize(size_t blobSize) {
  if (blobSize > getCurrentCumulativeBlocksizeLimit() - m_currency.minerTxBlobReservedSize()) {
    logger(ERROR) << "transaction is too big " << blobSize << ", maximum allowed size is " <<
      (getCurrentCumulativeBlocksizeLimit() - m_currency.minerTxBlobReservedSize());
    return false;
  }

  return true;
}

bool Blockchain::haveTransaction(const crypto::hash_t &id) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  return m_transactionMap.find(id) != m_transactionMap.end();
}

bool Blockchain::have_tx_keyimg_as_spent(const crypto::key_image_t &key_im) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  return  m_spent_keys.find(key_im) != m_spent_keys.end();
}

uint32_t Blockchain::getHeight() {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  return static_cast<uint32_t>(m_blocks.size());
}

bool Blockchain::init(bool load_existing) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  if (!m_blocks.init()) {
    return false;
  }

  if (load_existing && !m_blocks.empty()) {
    logger(INFO, BRIGHT_WHITE) << "Loading blockchain...";
    BlockCacheSerializer loader(*this, Block::getHash(m_blocks.back().bl), logger.getLogger());
    loader.load(m_currency.blocksCacheFileName());

    if (!loader.loaded()) {
      logger(WARNING, BRIGHT_YELLOW) << "No actual blockchain cache found, rebuilding internal structures...";
      rebuildCache();
    }

    loadBlockchainIndices();
  } else {
    m_blocks.clear();
  }

  if (m_blocks.empty()) {
    logger(INFO, BRIGHT_WHITE)
      << "Blockchain not loaded, generating genesis block.";
    block_verification_context_t bvc = boost::value_initialized<block_verification_context_t>();
    pushBlock(m_currency.genesisBlock(), bvc);
    if (bvc.m_verifivation_failed) {
      logger(ERROR, BRIGHT_RED) << "Failed to add genesis block to blockchain";
      return false;
    }
  } else {
    crypto::hash_t firstBlockHash = Block::getHash(m_blocks[0].bl);
    if (!(firstBlockHash == m_currency.genesisBlockHash())) {
      logger(ERROR, BRIGHT_RED) << "Failed to init: genesis block mismatch. "
        "Probably you set --testnet flag with data "
        "dir with non-test blockchain or another "
        "network.";
      return false;
    }
  }

  update_next_comulative_size_limit();

  uint64_t timestamp_diff = time(NULL) - m_blocks.back().bl.timestamp;
  if (!m_blocks.back().bl.timestamp) {
    timestamp_diff = time(NULL) - CRYPTONOTE_CREATION_TIME;
  }

  logger(INFO, BRIGHT_GREEN)
    << "Blockchain initialized. last block: " << m_blocks.size() - 1 << ", "
    << Common::timeIntervalToString(timestamp_diff)
    << " time ago, current difficulty: " << getDifficultyForNextBlock();
  return true;
}

void Blockchain::rebuildCache() {
  std::chrono::steady_clock::time_point timePoint = std::chrono::steady_clock::now();
  m_blockIndex.clear();
  m_transactionMap.clear();
  m_spent_keys.clear();
  m_outputs.clear();
  m_multisignatureOutputs.clear();
  for (uint32_t b = 0; b < m_blocks.size(); ++b) {
    if (b % 1000 == 0) {
      logger(INFO, BRIGHT_WHITE) << "Height " << b << " of " << m_blocks.size();
    }
    const block_entry_t& block = m_blocks[b];
    crypto::hash_t blockHash = Block::getHash(block.bl);
    m_blockIndex.push(blockHash);
    for (uint16_t t = 0; t < block.transactions.size(); ++t) {
      const transaction_entry_t& transaction = block.transactions[t];
      crypto::hash_t transactionHash = getObjectHash(transaction.tx);
      transaction_index_t transactionIndex = { b, t };
      m_transactionMap.insert(std::make_pair(transactionHash, transactionIndex));

      // process inputs
      for (auto& i : transaction.tx.inputs) {
        if (i.type() == typeid(key_input_t)) {
          m_spent_keys.insert(::boost::get<key_input_t>(i).keyImage);
        } else if (i.type() == typeid(multi_signature_input_t)) {
          auto out = ::boost::get<multi_signature_input_t>(i);
          m_multisignatureOutputs[out.amount][out.outputIndex].isUsed = true;
        }
      }

      // process outputs
      for (uint16_t o = 0; o < transaction.tx.outputs.size(); ++o) {
        const auto& out = transaction.tx.outputs[o];
        if (out.target.type() == typeid(key_output_t)) {
          m_outputs[out.amount].push_back(std::make_pair<>(transactionIndex, o));
        } else if (out.target.type() == typeid(multi_signature_output_t)) {
          multisignature_output_usage_t usage = { transactionIndex, o, false };
          m_multisignatureOutputs[out.amount].push_back(usage);
        }
      }
    }
  }

  std::chrono::duration<double> duration = std::chrono::steady_clock::now() - timePoint;
  logger(INFO, BRIGHT_WHITE) << "Rebuilding internal structures took: " << duration.count();
}

bool Blockchain::storeCache() {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  logger(INFO, BRIGHT_WHITE) << "Saving blockchain...";
  BlockCacheSerializer ser(*this, getTailId(), logger.getLogger());
  if (!ser.save(m_currency.blocksCacheFileName())) {
    logger(ERROR, BRIGHT_RED) << "Failed to save blockchain cache";
    return false;
  }

  return true;
}

bool Blockchain::deinit() {
  storeCache();
  storeBlockchainIndices();
  assert(m_messageQueueList.empty());
  return true;
}

bool Blockchain::resetAndSetGenesisBlock(const block_t& b) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  m_blocks.clear();
  m_blockIndex.clear();
  m_transactionMap.clear();

  m_spent_keys.clear();
  m_alternative_chains.clear();
  m_outputs.clear();

  m_paymentIdIndex.clear();
  m_timestampIndex.clear();
  m_generatedTransactionsIndex.clear();
  m_orthanBlocksIndex.clear();

  block_verification_context_t bvc = boost::value_initialized<block_verification_context_t>();
  addNewBlock(b, bvc);
  return bvc.m_added_to_main_chain && !bvc.m_verifivation_failed;
}

crypto::hash_t Blockchain::getTailId(uint32_t& height) {
  assert(!m_blocks.empty());
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  height = getHeight() - 1;
  return getTailId();
}

crypto::hash_t Blockchain::getTailId() {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  return m_blocks.empty() ? NULL_HASH : m_blockIndex.getTailId();
}

std::vector<crypto::hash_t> Blockchain::buildSparseChain() {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  assert(m_blockIndex.size() != 0);
  return doBuildSparseChain(m_blockIndex.getTailId());
}

std::vector<crypto::hash_t> Blockchain::buildSparseChain(const crypto::hash_t& startBlockId) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  assert(haveBlock(startBlockId));
  return doBuildSparseChain(startBlockId);
}

std::vector<crypto::hash_t> Blockchain::doBuildSparseChain(const crypto::hash_t& startBlockId) const {
  assert(m_blockIndex.size() != 0);

  std::vector<crypto::hash_t> sparseChain;

  if (m_blockIndex.hasBlock(startBlockId)) {
    sparseChain = m_blockIndex.buildSparseChain(startBlockId);
  } else {
    assert(m_alternative_chains.count(startBlockId) > 0);

    std::vector<crypto::hash_t> alternativeChain;
    crypto::hash_t blockchainAncestor;
    for (auto it = m_alternative_chains.find(startBlockId); it != m_alternative_chains.end(); it = m_alternative_chains.find(blockchainAncestor)) {
      alternativeChain.emplace_back(it->first);
      blockchainAncestor = it->second.bl.previousBlockHash;
    }

    for (size_t i = 1; i <= alternativeChain.size(); i *= 2) {
      sparseChain.emplace_back(alternativeChain[i - 1]);
    }

    assert(!sparseChain.empty());
    assert(m_blockIndex.hasBlock(blockchainAncestor));
    std::vector<crypto::hash_t> sparseMainChain = m_blockIndex.buildSparseChain(blockchainAncestor);
    sparseChain.reserve(sparseChain.size() + sparseMainChain.size());
    std::copy(sparseMainChain.begin(), sparseMainChain.end(), std::back_inserter(sparseChain));
  }

  return sparseChain;
}

crypto::hash_t Blockchain::getBlockIdByHeight(uint32_t height) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  assert(height < m_blockIndex.size());
  return m_blockIndex.getBlockId(height);
}

bool Blockchain::getBlockByHash(const crypto::hash_t& blockHash, block_t& b) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  uint32_t height = 0;

  if (m_blockIndex.getBlockHeight(blockHash, height)) {
    b = m_blocks[height].bl;
    return true;
  }

  logger(WARNING) << blockHash;

  auto blockByHashIterator = m_alternative_chains.find(blockHash);
  if (blockByHashIterator != m_alternative_chains.end()) {
    b = blockByHashIterator->second.bl;
    return true;
  }

  return false;
}

bool Blockchain::getBlockHeight(const crypto::hash_t& blockId, uint32_t& blockHeight) {
  std::lock_guard<decltype(m_blockchain_lock)> lock(m_blockchain_lock);
  return m_blockIndex.getBlockHeight(blockId, blockHeight);
}

difficulty_t Blockchain::getDifficultyForNextBlock() {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  std::vector<uint64_t> timestamps;
  std::vector<difficulty_t> commulative_difficulties;
  size_t offset = m_blocks.size() - std::min(m_blocks.size(), static_cast<uint64_t>(m_currency.difficultyBlocksCount()));
  if (offset == 0) {
    ++offset;
  }

  for (; offset < m_blocks.size(); offset++) {
    timestamps.push_back(m_blocks[offset].bl.timestamp);
    commulative_difficulties.push_back(m_blocks[offset].cumulative_difficulty);
  }

  return m_currency.nextDifficulty(timestamps, commulative_difficulties);
}

uint64_t Blockchain::getCoinsInCirculation() {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  if (m_blocks.empty()) {
    return 0;
  } else {
    return m_blocks.back().already_generated_coins;
  }
}

bool Blockchain::rollback_blockchain_switching(std::list<block_t> &original_chain, size_t rollback_height) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  // remove failed subchain
  for (size_t i = m_blocks.size() - 1; i >= rollback_height; i--) {
    popBlock(Block::getHash(m_blocks.back().bl));
  }

  // return back original chain
  for (auto &bl : original_chain) {
    block_verification_context_t bvc =
      boost::value_initialized<block_verification_context_t>();
    bool r = pushBlock(bl, bvc);
    if (!(r && bvc.m_added_to_main_chain)) {
      logger(ERROR, BRIGHT_RED) << "PANIC!!! failed to add (again) block while "
        "chain switching during the rollback!";
      return false;
    }
  }

  logger(INFO, BRIGHT_WHITE) << "Rollback success.";
  return true;
}

bool Blockchain::switch_to_alternative_blockchain(std::list<blocks_ext_by_hash_t::iterator>& alt_chain, bool discard_disconnected_chain) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  if (!(alt_chain.size())) {
    logger(ERROR, BRIGHT_RED) << "switch_to_alternative_blockchain: empty chain passed";
    return false;
  }

  size_t split_height = alt_chain.front()->second.height;

  if (!(m_blocks.size() > split_height)) {
    logger(ERROR, BRIGHT_RED) << "switch_to_alternative_blockchain: blockchain size is lower than split height";
    return false;
  }

  //disconnecting old chain
  std::list<block_t> disconnected_chain;
  for (size_t i = m_blocks.size() - 1; i >= split_height; i--) {
    block_t b = m_blocks[i].bl;
    popBlock(Block::getHash(b));
    //if (!(r)) { logger(ERROR, BRIGHT_RED) << "failed to remove block on chain switching"; return false; }
    disconnected_chain.push_front(b);
  }

  //connecting new alternative chain
  for (auto alt_ch_iter = alt_chain.begin(); alt_ch_iter != alt_chain.end(); alt_ch_iter++) {
    auto ch_ent = *alt_ch_iter;
    block_verification_context_t bvc = boost::value_initialized<block_verification_context_t>();
    bool r = pushBlock(ch_ent->second.bl, bvc);
    if (!r || !bvc.m_added_to_main_chain) {
      logger(INFO, BRIGHT_WHITE) << "Failed to switch to alternative blockchain";
      rollback_blockchain_switching(disconnected_chain, split_height);
      //add_block_as_invalid(ch_ent->second, Block::getHash(ch_ent->second.bl));
      logger(INFO, BRIGHT_WHITE) << "The block was inserted as invalid while connecting new alternative chain,  block_id: " << Block::getHash(ch_ent->second.bl);
      m_orthanBlocksIndex.remove(ch_ent->second.bl);
      m_alternative_chains.erase(ch_ent);

      for (auto alt_ch_to_orph_iter = ++alt_ch_iter; alt_ch_to_orph_iter != alt_chain.end(); alt_ch_to_orph_iter++) {
        //block_verification_context_t bvc = boost::value_initialized<block_verification_context_t>();
        //add_block_as_invalid((*alt_ch_iter)->second, (*alt_ch_iter)->first);
        m_orthanBlocksIndex.remove((*alt_ch_to_orph_iter)->second.bl);
        m_alternative_chains.erase(*alt_ch_to_orph_iter);
      }

      return false;
    }
  }

  if (!discard_disconnected_chain) {
    //pushing old chain as alternative chain
    for (auto& old_ch_ent : disconnected_chain) {
      block_verification_context_t bvc = boost::value_initialized<block_verification_context_t>();
      bool r = handle_alternative_block(old_ch_ent, Block::getHash(old_ch_ent), bvc, false);
      if (!r) {
        logger(ERROR, BRIGHT_RED) << ("Failed to push ex-main chain blocks to alternative chain ");
        rollback_blockchain_switching(disconnected_chain, split_height);
        return false;
      }
    }
  }

  std::vector<crypto::hash_t> blocksFromCommonRoot;
  blocksFromCommonRoot.reserve(alt_chain.size() + 1);
  blocksFromCommonRoot.push_back(alt_chain.front()->second.bl.previousBlockHash);

  //removing all_chain entries from alternative chain
  for (auto ch_ent : alt_chain) {
    blocksFromCommonRoot.push_back(Block::getHash(ch_ent->second.bl));
    m_orthanBlocksIndex.remove(ch_ent->second.bl);
    m_alternative_chains.erase(ch_ent);
  }

  sendMessage(BlockchainMessage(ChainSwitchMessage(std::move(blocksFromCommonRoot))));

  logger(INFO, BRIGHT_GREEN) << "REORGANIZE SUCCESS! on height: " << split_height << ", new blockchain size: " << m_blocks.size();
  return true;
}

difficulty_t Blockchain::get_next_difficulty_for_alternative_chain(const std::list<blocks_ext_by_hash_t::iterator>& alt_chain, block_entry_t& bei) {
  std::vector<uint64_t> timestamps;
  std::vector<difficulty_t> commulative_difficulties;
  if (alt_chain.size() < m_currency.difficultyBlocksCount()) {
    std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
    size_t main_chain_stop_offset = alt_chain.size() ? alt_chain.front()->second.height : bei.height;
    size_t main_chain_count = m_currency.difficultyBlocksCount() - std::min(m_currency.difficultyBlocksCount(), alt_chain.size());
    main_chain_count = std::min(main_chain_count, main_chain_stop_offset);
    size_t main_chain_start_offset = main_chain_stop_offset - main_chain_count;

    if (!main_chain_start_offset)
      ++main_chain_start_offset; //skip genesis block
    for (; main_chain_start_offset < main_chain_stop_offset; ++main_chain_start_offset) {
      timestamps.push_back(m_blocks[main_chain_start_offset].bl.timestamp);
      commulative_difficulties.push_back(m_blocks[main_chain_start_offset].cumulative_difficulty);
    }

    if (!((alt_chain.size() + timestamps.size()) <= m_currency.difficultyBlocksCount())) {
      logger(ERROR, BRIGHT_RED) << "Internal error, alt_chain.size()[" << alt_chain.size() << "] + timestamps.size()[" << timestamps.size() <<
        "] NOT <= m_currency.difficultyBlocksCount()[" << m_currency.difficultyBlocksCount() << ']'; return false;
    }
    for (auto it : alt_chain) {
      timestamps.push_back(it->second.bl.timestamp);
      commulative_difficulties.push_back(it->second.cumulative_difficulty);
    }
  } else {
    timestamps.resize(std::min(alt_chain.size(), m_currency.difficultyBlocksCount()));
    commulative_difficulties.resize(std::min(alt_chain.size(), m_currency.difficultyBlocksCount()));
    size_t count = 0;
    size_t max_i = timestamps.size() - 1;
    BOOST_REVERSE_FOREACH(auto it, alt_chain) {
      timestamps[max_i - count] = it->second.bl.timestamp;
      commulative_difficulties[max_i - count] = it->second.cumulative_difficulty;
      count++;
      if (count >= m_currency.difficultyBlocksCount()) {
        break;
      }
    }
  }

  return m_currency.nextDifficulty(timestamps, commulative_difficulties);
}

bool Blockchain::prevalidate_miner_transaction(const block_t& b, uint32_t height) {

  if (!(b.baseTransaction.inputs.size() == 1)) {
    logger(ERROR, BRIGHT_RED)
      << "coinbase transaction in the block has no inputs";
    return false;
  }

  if (!(b.baseTransaction.inputs[0].type() == typeid(base_input_t))) {
    logger(ERROR, BRIGHT_RED)
      << "coinbase transaction in the block has the wrong type";
    return false;
  }

  if (boost::get<base_input_t>(b.baseTransaction.inputs[0]).blockIndex != height) {
    logger(INFO, BRIGHT_RED) << "The miner transaction in block has invalid height: " <<
      boost::get<base_input_t>(b.baseTransaction.inputs[0]).blockIndex << ", expected: " << height;
    return false;
  }

  if (!(b.baseTransaction.unlockTime == height + m_currency.minedMoneyUnlockWindow())) {
    logger(ERROR, BRIGHT_RED)
      << "coinbase transaction transaction have wrong unlock time="
      << b.baseTransaction.unlockTime << ", expected "
      << height + m_currency.minedMoneyUnlockWindow();
    return false;
  }

  if (!check_outs_overflow(b.baseTransaction)) {
    logger(INFO, BRIGHT_RED) << "miner transaction have money overflow in block " << Block::getHash(b);
    return false;
  }

  return true;
}

bool Blockchain::validate_miner_transaction(const block_t& b, uint32_t height, size_t cumulativeBlockSize,
  uint64_t alreadyGeneratedCoins, uint64_t fee,
  uint64_t& reward, int64_t& emissionChange) {
  uint64_t minerReward = 0;
  for (auto& o : b.baseTransaction.outputs) {
    minerReward += o.amount;
  }

  std::vector<size_t> lastBlocksSizes;
  get_last_n_blocks_sizes(lastBlocksSizes, m_currency.rewardBlocksWindow());
  size_t blocksSizeMedian = Common::medianValue(lastBlocksSizes);

  if (!m_currency.getBlockReward(blocksSizeMedian, cumulativeBlockSize, alreadyGeneratedCoins, fee, reward, emissionChange)) {
    logger(INFO, BRIGHT_WHITE) << "block size " << cumulativeBlockSize << " is bigger than allowed for this blockchain";
    return false;
  }

  if (minerReward > reward) {
    logger(ERROR, BRIGHT_RED) << "Coinbase transaction spend too much money: " << m_currency.formatAmount(minerReward) <<
      ", block reward is " << m_currency.formatAmount(reward);
    return false;
  } else if (minerReward < reward) {
    logger(ERROR, BRIGHT_RED) << "Coinbase transaction doesn't use full amount of block reward: spent " <<
      m_currency.formatAmount(minerReward) << ", block reward is " << m_currency.formatAmount(reward);
    return false;
  }

  return true;
}

bool Blockchain::getBackwardBlocksSize(size_t from_height, std::vector<size_t>& sz, size_t count) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  if (!(from_height < m_blocks.size())) {
    logger(ERROR, BRIGHT_RED)
      << "Internal error: get_backward_blocks_sizes called with from_height="
      << from_height << ", blockchain height = " << m_blocks.size();
    return false;
  }
  size_t start_offset = (from_height + 1) - std::min((from_height + 1), count);
  for (size_t i = start_offset; i != from_height + 1; i++) {
    sz.push_back(m_blocks[i].block_cumulative_size);
  }

  return true;
}

bool Blockchain::get_last_n_blocks_sizes(std::vector<size_t>& sz, size_t count) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  if (!m_blocks.size()) {
    return true;
  }

  return getBackwardBlocksSize(m_blocks.size() - 1, sz, count);
}

uint64_t Blockchain::getCurrentCumulativeBlocksizeLimit() {
  return m_current_block_cumul_sz_limit;
}

bool Blockchain::complete_timestamps_vector(uint64_t start_top_height, std::vector<uint64_t>& timestamps) {
  if (timestamps.size() >= m_currency.timestampCheckWindow())
    return true;

  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  size_t need_elements = m_currency.timestampCheckWindow() - timestamps.size();
  if (!(start_top_height < m_blocks.size())) { logger(ERROR, BRIGHT_RED) << "internal error: passed start_height = " << start_top_height << " not less then m_blocks.size()=" << m_blocks.size(); return false; }
  size_t stop_offset = start_top_height > need_elements ? start_top_height - need_elements : 0;
  do {
    timestamps.push_back(m_blocks[start_top_height].bl.timestamp);
    if (start_top_height == 0)
      break;
    --start_top_height;
  } while (start_top_height != stop_offset);
  return true;
}

bool Blockchain::handle_alternative_block(const block_t& b, const crypto::hash_t& id, block_verification_context_t& bvc, bool sendNewAlternativeBlockMessage) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  auto block_height = get_block_height(b);
  if (block_height == 0) {
    logger(ERROR, BRIGHT_RED) <<
      "Block with id: " << Common::podToHex(id) << " (as alternative) have wrong miner transaction";
    bvc.m_verifivation_failed = true;
    return false;
  }

  if (!m_checkpoints.isAllowed(getHeight(), block_height)) {
    logger(TRACE) << "Block with id: " << id << std::endl <<
      " can't be accepted for alternative chain, block height: " << block_height << std::endl <<
      " blockchain height: " << getHeight();
    bvc.m_verifivation_failed = true;
    return false;
  }

  size_t cumulativeSize;
  if (!getBlockCumulativeSize(b, cumulativeSize)) {
    logger(TRACE) << "Block with id: " << id << " has at least one unknown transaction. Cumulative size is calculated imprecisely";
  }

  if (!checkCumulativeBlockSize(id, cumulativeSize, block_height)) {
    bvc.m_verifivation_failed = true;
    return false;
  }

  //block is not related with head of main chain
  //first of all - look in alternative chains container
  uint32_t mainPrevHeight = 0;
  const bool mainPrev = m_blockIndex.getBlockHeight(b.previousBlockHash, mainPrevHeight);
  const auto it_prev = m_alternative_chains.find(b.previousBlockHash);

  if (it_prev != m_alternative_chains.end() || mainPrev) {
    //we have new block in alternative chain

    //build alternative subchain, front -> mainchain, back -> alternative head
    blocks_ext_by_hash_t::iterator alt_it = it_prev; //m_alternative_chains.find()
    std::list<blocks_ext_by_hash_t::iterator> alt_chain;
    std::vector<uint64_t> timestamps;
    while (alt_it != m_alternative_chains.end()) {
      alt_chain.push_front(alt_it);
      timestamps.push_back(alt_it->second.bl.timestamp);
      alt_it = m_alternative_chains.find(alt_it->second.bl.previousBlockHash);
    }

    if (alt_chain.size()) {
      //make sure that it has right connection to main chain
      if (!(m_blocks.size() > alt_chain.front()->second.height)) { logger(ERROR, BRIGHT_RED) << "main blockchain wrong height"; return false; }
      crypto::hash_t h = NULL_HASH;
      Block::getHash(m_blocks[alt_chain.front()->second.height - 1].bl, h);
      if (!(h == alt_chain.front()->second.bl.previousBlockHash)) { logger(ERROR, BRIGHT_RED) << "alternative chain have wrong connection to main chain"; return false; }
      complete_timestamps_vector(alt_chain.front()->second.height - 1, timestamps);
    } else {
      if (!(mainPrev)) { logger(ERROR, BRIGHT_RED) << "internal error: broken imperative condition it_main_prev != m_blocks_index.end()"; return false; }
      complete_timestamps_vector(mainPrevHeight, timestamps);
    }

    //check timestamp correct
    if (!check_block_timestamp(timestamps, b)) {
      logger(INFO, BRIGHT_RED) <<
        "Block with id: " << id
        << ENDL << " for alternative chain, have invalid timestamp: " << b.timestamp;
      //add_block_as_invalid(b, id);//do not add blocks to invalid storage before proof of work check was passed
      bvc.m_verifivation_failed = true;
      return false;
    }

    block_entry_t bei = boost::value_initialized<block_entry_t>();
    bei.bl = b;
    bei.height = static_cast<uint32_t>(alt_chain.size() ? it_prev->second.height + 1 : mainPrevHeight + 1);

    bool is_a_checkpoint;
    if (!m_checkpoints.check(bei.height, id, is_a_checkpoint)) {
      logger(ERROR, BRIGHT_RED) <<
        "CHECKPOINT VALIDATION FAILED";
      bvc.m_verifivation_failed = true;
      return false;
    }

    // Always check PoW for alternative blocks
    m_is_in_checkpoint_zone = false;
    difficulty_t current_diff = get_next_difficulty_for_alternative_chain(alt_chain, bei);
    if (!(current_diff)) { logger(ERROR, BRIGHT_RED) << "!!!!!!! DIFFICULTY OVERHEAD !!!!!!!"; return false; }
    crypto::hash_t proof_of_work = NULL_HASH;
    if (!Block::checkProofOfWork(bei.bl, current_diff, proof_of_work)) {
      logger(INFO, BRIGHT_RED) <<
        "Block with id: " << id
        << ENDL << " for alternative chain, have not enough proof of work: " << proof_of_work
        << ENDL << " expected difficulty: " << current_diff;
      bvc.m_verifivation_failed = true;
      return false;
    }

    if (!prevalidate_miner_transaction(b, bei.height)) {
      logger(INFO, BRIGHT_RED) <<
        "Block with id: " << Common::podToHex(id) << " (as alternative) have wrong miner transaction.";
      bvc.m_verifivation_failed = true;
      return false;
    }

    bei.cumulative_difficulty = alt_chain.size() ? it_prev->second.cumulative_difficulty : m_blocks[mainPrevHeight].cumulative_difficulty;
    bei.cumulative_difficulty += current_diff;

#ifdef _DEBUG
    auto i_dres = m_alternative_chains.find(id);
    if (!(i_dres == m_alternative_chains.end())) { logger(ERROR, BRIGHT_RED) << "insertion of new alternative block returned as it already exist"; return false; }
#endif

    auto i_res = m_alternative_chains.insert(blocks_ext_by_hash_t::value_type(id, bei));
    if (!(i_res.second)) { logger(ERROR, BRIGHT_RED) << "insertion of new alternative block returned as it already exist"; return false; }

    m_orthanBlocksIndex.add(bei.bl);

    alt_chain.push_back(i_res.first);

    if (is_a_checkpoint) {
      //do reorganize!
      logger(INFO, BRIGHT_GREEN) <<
        "###### REORGANIZE on height: " << alt_chain.front()->second.height << " of " << m_blocks.size() - 1 <<
        ", checkpoint is found in alternative chain on height " << bei.height;
      bool r = switch_to_alternative_blockchain(alt_chain, true);
      if (r) {
        bvc.m_added_to_main_chain = true;
        bvc.m_switched_to_alt_chain = true;
      } else {
        bvc.m_verifivation_failed = true;
      }
      return r;
    } else if (m_blocks.back().cumulative_difficulty < bei.cumulative_difficulty) //check if difficulty bigger then in main chain
    {
      //do reorganize!
      logger(INFO, BRIGHT_GREEN) <<
        "###### REORGANIZE on height: " << alt_chain.front()->second.height << " of " << m_blocks.size() - 1 << " with cum_difficulty " << m_blocks.back().cumulative_difficulty
        << ENDL << " alternative blockchain size: " << alt_chain.size() << " with cum_difficulty " << bei.cumulative_difficulty;
      bool r = switch_to_alternative_blockchain(alt_chain, false);
      if (r) {
        bvc.m_added_to_main_chain = true;
        bvc.m_switched_to_alt_chain = true;
      } else {
        bvc.m_verifivation_failed = true;
      }
      return r;
    } else {
      logger(INFO, BRIGHT_BLUE) <<
        "----- BLOCK ADDED AS ALTERNATIVE ON HEIGHT " << bei.height
        << ENDL << "id:\t" << id
        << ENDL << "PoW:\t" << proof_of_work
        << ENDL << "difficulty:\t" << current_diff;
      if (sendNewAlternativeBlockMessage) {
        sendMessage(BlockchainMessage(NewAlternativeBlockMessage(id)));
      }
      return true;
    }
  } else {
    //block orphaned
    bvc.m_marked_as_orphaned = true;
    logger(INFO, BRIGHT_RED) <<
      "Block recognized as orphaned and rejected, id = " << id;
  }

  return true;
}

bool Blockchain::getBlocks(uint32_t start_offset, uint32_t count, std::list<block_t>& blocks, std::list<transaction_t>& txs) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  if (start_offset >= m_blocks.size())
    return false;
  for (size_t i = start_offset; i < start_offset + count && i < m_blocks.size(); i++) {
    blocks.push_back(m_blocks[i].bl);
    std::list<crypto::hash_t> missed_ids;
    getTransactions(m_blocks[i].bl.transactionHashes, txs, missed_ids);
    if (!(!missed_ids.size())) { logger(ERROR, BRIGHT_RED) << "have missed transactions in own block in main blockchain"; return false; }
  }

  return true;
}

bool Blockchain::getBlocks(uint32_t start_offset, uint32_t count, std::list<block_t>& blocks) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  if (start_offset >= m_blocks.size()) {
    return false;
  }

  for (uint32_t i = start_offset; i < start_offset + count && i < m_blocks.size(); i++) {
    blocks.push_back(m_blocks[i].bl);
  }

  return true;
}

bool Blockchain::handleGetObjects(NOTIFY_REQUEST_GET_OBJECTS::request& arg, NOTIFY_RESPONSE_GET_OBJECTS::request& rsp) { //Deprecated. Should be removed with CryptoNoteProtocolHandler.
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  rsp.current_blockchain_height = getHeight();
  std::list<block_t> blocks;
  getBlocks(arg.blocks, blocks, rsp.missed_ids);

  for (const auto& bl : blocks) {
    std::list<crypto::hash_t> missed_tx_id;
    std::list<transaction_t> txs;
    getTransactions(bl.transactionHashes, txs, rsp.missed_ids);
    if (!(!missed_tx_id.size())) { logger(ERROR, BRIGHT_RED) << "Internal error: have missed missed_tx_id.size()=" << missed_tx_id.size() << ENDL << "for block id = " << Block::getHash(bl); return false; } //WTF???
    rsp.blocks.push_back(block_complete_entry_t());
    block_complete_entry_t& e = rsp.blocks.back();
    //pack block
    e.block = asString(BinaryArray::to(bl));
    //pack transactions
    for (transaction_t& tx : txs) {
      e.txs.push_back(asString(BinaryArray::to(tx)));
    }
  }

  //get another transactions, if need
  std::list<transaction_t> txs;
  getTransactions(arg.txs, txs, rsp.missed_ids);
  //pack aside transactions
  for (const auto& tx : txs) {
    rsp.txs.push_back(asString(BinaryArray::to(tx)));
  }

  return true;
}

bool Blockchain::getAlternativeBlocks(std::list<block_t>& blocks) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  for (auto& alt_bl : m_alternative_chains) {
    blocks.push_back(alt_bl.second.bl);
  }

  return true;
}

uint32_t Blockchain::getAlternativeBlocksCount() {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  return static_cast<uint32_t>(m_alternative_chains.size());
}

bool Blockchain::add_out_to_get_random_outs(std::vector<std::pair<transaction_index_t, uint16_t>>& amount_outs, COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount& result_outs, uint64_t amount, size_t i) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  const transaction_t& tx = transactionByIndex(amount_outs[i].first).tx;
  if (!(tx.outputs.size() > amount_outs[i].second)) {
    logger(ERROR, BRIGHT_RED) << "internal error: in global outs index, transaction out index="
      << amount_outs[i].second << " more than transaction outputs = " << tx.outputs.size() << ", for tx id = " << getObjectHash(tx); return false;
  }
  if (!(tx.outputs[amount_outs[i].second].target.type() == typeid(key_output_t))) { logger(ERROR, BRIGHT_RED) << "unknown tx out type"; return false; }

  //check if transaction is unlocked
  if (!is_tx_spendtime_unlocked(tx.unlockTime))
    return false;

  COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::out_entry& oen = *result_outs.outs.insert(result_outs.outs.end(), COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::out_entry());
  oen.global_amount_index = static_cast<uint32_t>(i);
  oen.out_key = boost::get<key_output_t>(tx.outputs[amount_outs[i].second].target).key;
  return true;
}

size_t Blockchain::find_end_of_allowed_index(const std::vector<std::pair<transaction_index_t, uint16_t>>& amount_outs) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  if (amount_outs.empty()) {
    return 0;
  }

  size_t i = amount_outs.size();
  do {
    --i;
    if (amount_outs[i].first.block + m_currency.minedMoneyUnlockWindow() <= getHeight()) {
      return i + 1;
    }
  } while (i != 0);

  return 0;
}

bool Blockchain::getRandomOutsByAmount(const COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::request& req, COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::response& res) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  for (uint64_t amount : req.amounts) {
    COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount& result_outs = *res.outs.insert(res.outs.end(), COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount());
    result_outs.amount = amount;
    auto it = m_outputs.find(amount);
    if (it == m_outputs.end()) {
      logger(ERROR, BRIGHT_RED) <<
        "COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS: not outs for amount " << amount << ", wallet should use some real outs when it lookup for some mix, so, at least one out for this amount should exist";
      continue;//actually this is strange situation, wallet should use some real outs when it lookup for some mix, so, at least one out for this amount should exist
    }

    std::vector<std::pair<transaction_index_t, uint16_t>>& amount_outs = it->second;
    //it is not good idea to use top fresh outs, because it increases possibility of transaction canceling on split
    //lets find upper bound of not fresh outs
    size_t up_index_limit = find_end_of_allowed_index(amount_outs);
    if (!(up_index_limit <= amount_outs.size())) { logger(ERROR, BRIGHT_RED) << "internal error: find_end_of_allowed_index returned wrong index=" << up_index_limit << ", with amount_outs.size = " << amount_outs.size(); return false; }

    if (up_index_limit > 0) {
      ShuffleGenerator<size_t, crypto::random_engine<size_t>> generator(up_index_limit);
      for (uint64_t j = 0; j < up_index_limit && result_outs.outs.size() < req.outs_count; ++j) {
        add_out_to_get_random_outs(amount_outs, result_outs, amount, generator());
      }
    }
  }
  return true;
}

uint32_t Blockchain::findBlockchainSupplement(const std::vector<crypto::hash_t>& qblock_ids) {
  assert(!qblock_ids.empty());
  assert(qblock_ids.back() == m_blockIndex.getBlockId(0));

  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  uint32_t blockIndex;
  // assert above guarantees that method returns true
  m_blockIndex.findSupplement(qblock_ids, blockIndex);
  return blockIndex;
}

uint64_t Blockchain::blockDifficulty(size_t i) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  if (!(i < m_blocks.size())) { logger(ERROR, BRIGHT_RED) << "wrong block index i = " << i << " at Blockchain::block_difficulty()"; return false; }
  if (i == 0)
    return m_blocks[i].cumulative_difficulty;

  return m_blocks[i].cumulative_difficulty - m_blocks[i - 1].cumulative_difficulty;
}

void Blockchain::print_blockchain(uint64_t start_index, uint64_t end_index) {
  std::stringstream ss;
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  if (start_index >= m_blocks.size()) {
    logger(INFO, BRIGHT_WHITE) <<
      "Wrong starter index set: " << start_index << ", expected max index " << m_blocks.size() - 1;
    return;
  }

  for (size_t i = start_index; i != m_blocks.size() && i != end_index; i++) {
    ss << Block::toString(m_blocks[i]);
    ss << "Difficulty: " << blockDifficulty(i) << std::endl;
  }
  logger(DEBUGGING) <<
    "Current blockchain:" << ENDL << ss.str();
  logger(INFO, BRIGHT_WHITE) <<
    "Blockchain printed with log level 1";
}

void Blockchain::print_blockchain_index() {
  std::stringstream ss;
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  std::vector<crypto::hash_t> blockIds = m_blockIndex.getBlockIds(0, std::numeric_limits<uint32_t>::max());
  logger(INFO, BRIGHT_WHITE) << "Current blockchain index:";

  size_t height = 0;
  for (auto i = blockIds.begin(); i != blockIds.end(); ++i, ++height) {
    logger(INFO, BRIGHT_WHITE) << "id\t\t" << *i << " height" << height;
  }

}

void Blockchain::print_blockchain_outs(const std::string& file) {
  std::stringstream ss;
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  for (const outputs_container_t::value_type& v : m_outputs) {
    const std::vector<std::pair<transaction_index_t, uint16_t>>& vals = v.second;
    if (!vals.empty()) {
      ss << "amount: " << v.first << ENDL;
      for (size_t i = 0; i != vals.size(); i++) {
        ss << "\t" << getObjectHash(transactionByIndex(vals[i].first).tx) << ": " << vals[i].second << ENDL;
      }
    }
  }

  if (Common::saveStringToFile(file, ss.str())) {
    logger(INFO, BRIGHT_WHITE) <<
      "Current outputs index writen to file: " << file;
  } else {
    logger(WARNING, BRIGHT_YELLOW) <<
      "Failed to write current outputs index to file: " << file;
  }
}

std::vector<crypto::hash_t> Blockchain::findBlockchainSupplement(const std::vector<crypto::hash_t>& remoteBlockIds, size_t maxCount,
  uint32_t& totalBlockCount, uint32_t& startBlockIndex) {

  assert(!remoteBlockIds.empty());
  assert(remoteBlockIds.back() == m_blockIndex.getBlockId(0));

  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  totalBlockCount = getHeight();
  startBlockIndex = findBlockchainSupplement(remoteBlockIds);

  return m_blockIndex.getBlockIds(startBlockIndex, static_cast<uint32_t>(maxCount));
}

bool Blockchain::haveBlock(const crypto::hash_t& id) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  if (m_blockIndex.hasBlock(id))
    return true;

  if (m_alternative_chains.count(id))
    return true;

  return false;
}

size_t Blockchain::getTotalTransactions() {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  return m_transactionMap.size();
}

bool Blockchain::getTransactionOutputGlobalIndexes(const crypto::hash_t& tx_id, std::vector<uint32_t>& indexs) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  auto it = m_transactionMap.find(tx_id);
  if (it == m_transactionMap.end()) {
    logger(WARNING, YELLOW) << "warning: get_tx_outputs_gindexs failed to find transaction with id = " << tx_id;
    return false;
  }

  const transaction_entry_t& tx = transactionByIndex(it->second);
  if (!(tx.m_global_output_indexes.size())) { logger(ERROR, BRIGHT_RED) << "internal error: global indexes for transaction " << tx_id << " is empty"; return false; }
  indexs.resize(tx.m_global_output_indexes.size());
  for (size_t i = 0; i < tx.m_global_output_indexes.size(); ++i) {
    indexs[i] = tx.m_global_output_indexes[i];
  }

  return true;
}

bool Blockchain::get_out_by_msig_gindex(uint64_t amount, uint64_t gindex, multi_signature_output_t& out) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  auto it = m_multisignatureOutputs.find(amount);
  if (it == m_multisignatureOutputs.end()) {
    return false;
  }

  if (it->second.size() <= gindex) {
    return false;
  }

  auto msigUsage = it->second[gindex];
  auto& targetOut = transactionByIndex(msigUsage.transactionIndex).tx.outputs[msigUsage.outputIndex].target;
  if (targetOut.type() != typeid(multi_signature_output_t)) {
    return false;
  }

  out = boost::get<multi_signature_output_t>(targetOut);
  return true;
}



bool Blockchain::checkTransactionInputs(const transaction_t& tx, uint32_t& max_used_block_height, crypto::hash_t& max_used_block_id, block_info_t* tail) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  if (tail)
    tail->id = getTailId(tail->height);

  bool res = checkTransactionInputs(tx, &max_used_block_height);
  if (!res) return false;
  if (!(max_used_block_height < m_blocks.size())) { logger(ERROR, BRIGHT_RED) << "internal error: max used block index=" << max_used_block_height << " is not less then blockchain size = " << m_blocks.size(); return false; }
  Block::getHash(m_blocks[max_used_block_height].bl, max_used_block_id);
  return true;
}

bool Blockchain::haveTransactionKeyImagesAsSpent(const transaction_t &tx) {
  for (const auto& in : tx.inputs) {
    if (in.type() == typeid(key_input_t)) {
      if (have_tx_keyimg_as_spent(boost::get<key_input_t>(in).keyImage)) {
        return true;
      }
    }
  }

  return false;
}

bool Blockchain::checkTransactionInputs(const transaction_t& tx, uint32_t* pmax_used_block_height) {
  crypto::hash_t tx_prefix_hash = getObjectHash(*static_cast<const transaction_prefix_t*>(&tx));
  return checkTransactionInputs(tx, tx_prefix_hash, pmax_used_block_height);
}

bool Blockchain::checkTransactionInputs(const transaction_t& tx, const crypto::hash_t& tx_prefix_hash, uint32_t* pmax_used_block_height) {
  size_t inputIndex = 0;
  if (pmax_used_block_height) {
    *pmax_used_block_height = 0;
  }

  crypto::hash_t transactionHash = getObjectHash(tx);
  for (const auto& txin : tx.inputs) {
    assert(inputIndex < tx.signatures.size());
    if (txin.type() == typeid(key_input_t)) {
      const key_input_t& in_to_key = boost::get<key_input_t>(txin);
      if (!(!in_to_key.outputIndexes.empty())) { logger(ERROR, BRIGHT_RED) << "empty in_to_key.outputIndexes in transaction with id " << getObjectHash(tx); return false; }

      if (have_tx_keyimg_as_spent(in_to_key.keyImage)) {
        logger(DEBUGGING) <<
          "Key image already spent in blockchain: " << Common::podToHex(in_to_key.keyImage);
        return false;
      }

      if (!check_tx_input(in_to_key, tx_prefix_hash, tx.signatures[inputIndex], pmax_used_block_height)) {
        logger(INFO, BRIGHT_WHITE) <<
          "Failed to check ring signature for tx " << transactionHash;
        return false;
      }

      ++inputIndex;
    } else if (txin.type() == typeid(multi_signature_input_t)) {
      if (!validateInput(::boost::get<multi_signature_input_t>(txin), transactionHash, tx_prefix_hash, tx.signatures[inputIndex])) {
        return false;
      }

      ++inputIndex;
    } else {
      logger(INFO, BRIGHT_WHITE) <<
        "transaction_t << " << transactionHash << " contains input of unsupported type.";
      return false;
    }
  }

  return true;
}

bool Blockchain::is_tx_spendtime_unlocked(uint64_t unlock_time) {
  if (unlock_time < m_currency.maxBlockHeight()) {
    //interpret as block index
    if (getHeight() - 1 + m_currency.lockedTxAllowedDeltaBlocks() >= unlock_time)
      return true;
    else
      return false;
  } else {
    //interpret as time
    uint64_t current_time = static_cast<uint64_t>(time(NULL));
    if (current_time + m_currency.lockedTxAllowedDeltaSeconds() >= unlock_time)
      return true;
    else
      return false;
  }

  return false;
}

bool Blockchain::check_tx_input(const key_input_t& txin, const crypto::hash_t& tx_prefix_hash, const std::vector<crypto::signature_t>& sig, uint32_t* pmax_related_block_height) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  struct outputs_visitor {
    std::vector<const crypto::public_key_t *>& m_results_collector;
    Blockchain& m_bch;
    LoggerRef logger;
    outputs_visitor(std::vector<const crypto::public_key_t *>& results_collector, Blockchain& bch, ILogger& logger) :m_results_collector(results_collector), m_bch(bch), logger(logger, "outputs_visitor") {
    }

    bool handle_output(const transaction_t& tx, const transaction_output_t& out, size_t transactionOutputIndex) {
      //check tx unlock time
      if (!m_bch.is_tx_spendtime_unlocked(tx.unlockTime)) {
        logger(INFO, BRIGHT_WHITE) <<
          "One of outputs for one of inputs have wrong tx.unlockTime = " << tx.unlockTime;
        return false;
      }

      if (out.target.type() != typeid(key_output_t)) {
        logger(INFO, BRIGHT_WHITE) <<
          "Output have wrong type id, which=" << out.target.which();
        return false;
      }

      m_results_collector.push_back(&boost::get<key_output_t>(out.target).key);
      return true;
    }
  };

  //check ring signature
  std::vector<const crypto::public_key_t *> output_keys;
  outputs_visitor vi(output_keys, *this, logger.getLogger());
  if (!scanOutputKeysForIndexes(txin, vi, pmax_related_block_height)) {
    logger(INFO, BRIGHT_WHITE) <<
      "Failed to get output keys for tx with amount = " << m_currency.formatAmount(txin.amount) <<
      " and count indexes " << txin.outputIndexes.size();
    return false;
  }

  if (txin.outputIndexes.size() != output_keys.size()) {
    logger(INFO, BRIGHT_WHITE) <<
      "Output keys for tx with amount = " << txin.amount << " and count indexes " << txin.outputIndexes.size() << " returned wrong keys count " << output_keys.size();
    return false;
  }

  if (!(sig.size() == output_keys.size())) { logger(ERROR, BRIGHT_RED) << "internal error: tx signatures count=" << sig.size() << " mismatch with outputs keys count for inputs=" << output_keys.size(); return false; }
  if (m_is_in_checkpoint_zone) {
    return true;
  }

  return crypto::check_ring_signature(tx_prefix_hash, txin.keyImage, output_keys, sig.data());
}

uint64_t Blockchain::get_adjusted_time() {
  //TODO: add collecting median time
  return time(NULL);
}

bool Blockchain::check_block_timestamp_main(const block_t& b) {
  if (b.timestamp > get_adjusted_time() + m_currency.blockFutureTimeLimit()) {
    logger(INFO, BRIGHT_WHITE) <<
      "Timestamp of block with id: " << Block::getHash(b) << ", " << b.timestamp << ", bigger than adjusted time + 2 hours";
    return false;
  }

  std::vector<uint64_t> timestamps;
  size_t offset = m_blocks.size() <= m_currency.timestampCheckWindow() ? 0 : m_blocks.size() - m_currency.timestampCheckWindow();
  for (; offset != m_blocks.size(); ++offset) {
    timestamps.push_back(m_blocks[offset].bl.timestamp);
  }

  return check_block_timestamp(std::move(timestamps), b);
}

bool Blockchain::check_block_timestamp(std::vector<uint64_t> timestamps, const block_t& b) {
  if (timestamps.size() < m_currency.timestampCheckWindow()) {
    return true;
  }

  uint64_t median_ts = Common::medianValue(timestamps);

  if (b.timestamp < median_ts) {
    logger(INFO, BRIGHT_WHITE) <<
      "Timestamp of block with id: " << Block::getHash(b) << ", " << b.timestamp <<
      ", less than median of last " << m_currency.timestampCheckWindow() << " blocks, " << median_ts;
    return false;
  }

  return true;
}

bool Blockchain::checkCumulativeBlockSize(const crypto::hash_t& blockId, size_t cumulativeBlockSize, uint64_t height) {
  size_t maxBlockCumulativeSize = m_currency.maxBlockCumulativeSize(height);
  if (cumulativeBlockSize > maxBlockCumulativeSize) {
    logger(INFO, BRIGHT_WHITE) <<
      "Block " << blockId << " is too big: " << cumulativeBlockSize << " bytes, " <<
      "exptected no more than " << maxBlockCumulativeSize << " bytes";
    return false;
  }

  return true;
}

// Returns true, if cumulativeSize is calculated precisely, else returns false.
bool Blockchain::getBlockCumulativeSize(const block_t& block, size_t& cumulativeSize) {
  std::vector<transaction_t> blockTxs;
  std::vector<crypto::hash_t> missedTxs;
  getTransactions(block.transactionHashes, blockTxs, missedTxs, true);

  cumulativeSize = BinaryArray::size(block.baseTransaction);
  for (const transaction_t& tx : blockTxs) {
    cumulativeSize += BinaryArray::size(tx);
  }

  return missedTxs.empty();
}

// Precondition: m_blockchain_lock is locked.
bool Blockchain::update_next_comulative_size_limit() {
  std::vector<size_t> sz;
  get_last_n_blocks_sizes(sz, m_currency.rewardBlocksWindow());

  uint64_t median = Common::medianValue(sz);
  if (median <= m_currency.blockGrantedFullRewardZone()) {
    median = m_currency.blockGrantedFullRewardZone();
  }

  m_current_block_cumul_sz_limit = median * 2;
  return true;
}

bool Blockchain::addNewBlock(const block_t& bl_, block_verification_context_t& bvc) {
  //copy block here to let modify block.target
  block_t bl = bl_;
  crypto::hash_t id;
  if (!Block::getHash(bl, id)) {
    logger(ERROR, BRIGHT_RED) <<
      "Failed to get block hash, possible block has invalid format";
    bvc.m_verifivation_failed = true;
    return false;
  }

  bool add_result;

  { //to avoid deadlock lets lock tx_pool for whole add/reorganize process
    std::lock_guard<decltype(m_tx_pool)> poolLock(m_tx_pool);
    std::lock_guard<decltype(m_blockchain_lock)> bcLock(m_blockchain_lock);

    if (haveBlock(id)) {
      logger(TRACE) << "block with id = " << id << " already exists";
      bvc.m_already_exists = true;
      return false;
    }

    //check that block refers to chain tail
    if (!(bl.previousBlockHash == getTailId())) {
      //chain switching or wrong block
      bvc.m_added_to_main_chain = false;
      add_result = handle_alternative_block(bl, id, bvc);
    } else {
      add_result = pushBlock(bl, bvc);
      if (add_result) {
        sendMessage(BlockchainMessage(NewBlockMessage(id)));
      }
    }
  }

  if (add_result && bvc.m_added_to_main_chain) {
    m_observerManager.notify(&IBlockchainStorageObserver::blockchainUpdated);
  }

  return add_result;
}

const transaction_entry_t& Blockchain::transactionByIndex(transaction_index_t index) {
  return m_blocks[index.block].transactions[index.transaction];
}

bool Blockchain::pushBlock(const block_t& blockData, block_verification_context_t& bvc) {
  std::vector<transaction_t> transactions;
  if (!loadTransactions(blockData, transactions)) {
    bvc.m_verifivation_failed = true;
    return false;
  }

  if (!pushBlock(blockData, transactions, bvc)) {
    saveTransactions(transactions);
    return false;
  }

  return true;
}

bool Blockchain::pushBlock(const block_t& blockData, const std::vector<transaction_t>& transactions, block_verification_context_t& bvc) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  auto blockProcessingStart = std::chrono::steady_clock::now();

  crypto::hash_t blockHash = Block::getHash(blockData);

  if (m_blockIndex.hasBlock(blockHash)) {
    logger(ERROR, BRIGHT_RED) <<
      "Block " << blockHash << " already exists in blockchain.";
    bvc.m_verifivation_failed = true;
    return false;
  }

  if (blockData.previousBlockHash != getTailId()) {
    logger(INFO, BRIGHT_WHITE) <<
      "Block " << blockHash << " has wrong previousBlockHash: " << blockData.previousBlockHash << ", expected: " << getTailId();
    bvc.m_verifivation_failed = true;
    return false;
  }

  if (!check_block_timestamp_main(blockData)) {
    logger(INFO, BRIGHT_WHITE) <<
      "Block " << blockHash << " has invalid timestamp: " << blockData.timestamp;
    bvc.m_verifivation_failed = true;
    return false;
  }

  auto targetTimeStart = std::chrono::steady_clock::now();
  difficulty_t currentDifficulty = getDifficultyForNextBlock();
  auto target_calculating_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - targetTimeStart).count();

  if (!(currentDifficulty)) {
    logger(ERROR, BRIGHT_RED) << "!!!!!!!!! difficulty overhead !!!!!!!!!";
    return false;
  }


  auto longhashTimeStart = std::chrono::steady_clock::now();
  crypto::hash_t proof_of_work = NULL_HASH;
  if (m_checkpoints.isCheckpoint(getHeight())) {
    if (!m_checkpoints.check(getHeight(), blockHash)) {
      logger(ERROR, BRIGHT_RED) <<
        "CHECKPOINT VALIDATION FAILED";
      bvc.m_verifivation_failed = true;
      return false;
    }
  } else {
    if (!Block::checkProofOfWork(blockData, currentDifficulty, proof_of_work)) {
      logger(INFO, BRIGHT_WHITE) <<
        "Block " << blockHash << ", has too weak proof of work: " << proof_of_work << ", expected difficulty: " << currentDifficulty;
      bvc.m_verifivation_failed = true;
      return false;
    }
  }

  auto longhash_calculating_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - longhashTimeStart).count();

  if (!prevalidate_miner_transaction(blockData, static_cast<uint32_t>(m_blocks.size()))) {
    logger(INFO, BRIGHT_WHITE) <<
      "Block " << blockHash << " failed to pass prevalidation";
    bvc.m_verifivation_failed = true;
    return false;
  }

  crypto::hash_t minerTransactionHash = getObjectHash(blockData.baseTransaction);

  block_entry_t block;
  block.bl = blockData;
  block.transactions.resize(1);
  block.transactions[0].tx = blockData.baseTransaction;
  transaction_index_t transactionIndex = { static_cast<uint32_t>(m_blocks.size()), static_cast<uint16_t>(0) };
  pushTransaction(block, minerTransactionHash, transactionIndex);

  size_t coinbase_blob_size = BinaryArray::size(blockData.baseTransaction);
  size_t cumulative_block_size = coinbase_blob_size;
  uint64_t fee_summary = 0;
  for (size_t i = 0; i < transactions.size(); ++i) {
    const crypto::hash_t& tx_id = blockData.transactionHashes[i];
    block.transactions.resize(block.transactions.size() + 1);
    size_t blob_size = 0;
    uint64_t fee = 0;
    block.transactions.back().tx = transactions[i];

    blob_size = BinaryArray::to(block.transactions.back().tx).size();
    fee = getInputAmount(block.transactions.back().tx) - getOutputAmount(block.transactions.back().tx);
    if (!checkTransactionInputs(block.transactions.back().tx)) {
      logger(INFO, BRIGHT_WHITE) <<
        "Block " << blockHash << " has at least one transaction with wrong inputs: " << tx_id;
      bvc.m_verifivation_failed = true;

      block.transactions.pop_back();
      popTransactions(block, minerTransactionHash);
      return false;
    }

    ++transactionIndex.transaction;
    pushTransaction(block, tx_id, transactionIndex);

    cumulative_block_size += blob_size;
    fee_summary += fee;
  }

  if (!checkCumulativeBlockSize(blockHash, cumulative_block_size, m_blocks.size())) {
    bvc.m_verifivation_failed = true;
    return false;
  }

  int64_t emissionChange = 0;
  uint64_t reward = 0;
  uint64_t already_generated_coins = m_blocks.empty() ? 0 : m_blocks.back().already_generated_coins;
  if (!validate_miner_transaction(blockData, static_cast<uint32_t>(m_blocks.size()), cumulative_block_size, already_generated_coins, fee_summary, reward, emissionChange)) {
    logger(INFO, BRIGHT_WHITE) << "Block " << blockHash << " has invalid miner transaction";
    bvc.m_verifivation_failed = true;
    popTransactions(block, minerTransactionHash);
    return false;
  }

  block.height = static_cast<uint32_t>(m_blocks.size());
  block.block_cumulative_size = cumulative_block_size;
  block.cumulative_difficulty = currentDifficulty;
  block.already_generated_coins = already_generated_coins + emissionChange;
  if (m_blocks.size() > 0) {
    block.cumulative_difficulty += m_blocks.back().cumulative_difficulty;
  }

  pushBlock(block);

  auto block_processing_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - blockProcessingStart).count();

  logger(DEBUGGING) <<
    "+++++ BLOCK SUCCESSFULLY ADDED" << ENDL << "id:\t" << blockHash
    << ENDL << "PoW:\t" << proof_of_work
    << ENDL << "HEIGHT " << block.height << ", difficulty:\t" << currentDifficulty
    << ENDL << "block reward: " << m_currency.formatAmount(reward) << ", fee = " << m_currency.formatAmount(fee_summary)
    << ", coinbase_blob_size: " << coinbase_blob_size << ", cumulative size: " << cumulative_block_size
    << ", " << block_processing_time << "(" << target_calculating_time << "/" << longhash_calculating_time << ")ms";

  bvc.m_added_to_main_chain = true;

  update_next_comulative_size_limit();

  return true;
}

bool Blockchain::pushBlock(block_entry_t& block) {
  crypto::hash_t blockHash = Block::getHash(block.bl);

  m_blocks.push_back(block);
  m_blockIndex.push(blockHash);

  m_timestampIndex.add(block.bl.timestamp, blockHash);
  m_generatedTransactionsIndex.add(block.bl);

  assert(m_blockIndex.size() == m_blocks.size());

  return true;
}

void Blockchain::popBlock(const crypto::hash_t& blockHash) {
  if (m_blocks.empty()) {
    logger(ERROR, BRIGHT_RED) <<
      "Attempt to pop block from empty blockchain.";
    return;
  }

  std::vector<transaction_t> transactions(m_blocks.back().transactions.size() - 1);
  for (size_t i = 0; i < m_blocks.back().transactions.size() - 1; ++i) {
    transactions[i] = m_blocks.back().transactions[1 + i].tx;
  }

  saveTransactions(transactions);

  popTransactions(m_blocks.back(), getObjectHash(m_blocks.back().bl.baseTransaction));

  m_timestampIndex.remove(m_blocks.back().bl.timestamp, blockHash);
  m_generatedTransactionsIndex.remove(m_blocks.back().bl);

  m_blocks.pop_back();
  m_blockIndex.pop();

  assert(m_blockIndex.size() == m_blocks.size());
}

bool Blockchain::pushTransaction(block_entry_t& block, const crypto::hash_t& transactionHash, transaction_index_t transactionIndex) {
  auto result = m_transactionMap.insert(std::make_pair(transactionHash, transactionIndex));
  if (!result.second) {
    logger(ERROR, BRIGHT_RED) <<
      "Duplicate transaction was pushed to blockchain.";
    return false;
  }

  transaction_entry_t& transaction = block.transactions[transactionIndex.transaction];

  if (!checkMultisignatureInputsDiff(transaction.tx)) {
    logger(ERROR, BRIGHT_RED) <<
      "Double spending transaction was pushed to blockchain.";
    m_transactionMap.erase(transactionHash);
    return false;
  }

  for (size_t i = 0; i < transaction.tx.inputs.size(); ++i) {
    if (transaction.tx.inputs[i].type() == typeid(key_input_t)) {
      auto result = m_spent_keys.insert(::boost::get<key_input_t>(transaction.tx.inputs[i]).keyImage);
      if (!result.second) {
        logger(ERROR, BRIGHT_RED) <<
          "Double spending transaction was pushed to blockchain.";
        for (size_t j = 0; j < i; ++j) {
          m_spent_keys.erase(::boost::get<key_input_t>(transaction.tx.inputs[i - 1 - j]).keyImage);
        }

        m_transactionMap.erase(transactionHash);
        return false;
      }
    }
  }

  for (const auto& inv : transaction.tx.inputs) {
    if (inv.type() == typeid(multi_signature_input_t)) {
      const multi_signature_input_t& in = ::boost::get<multi_signature_input_t>(inv);
      auto& amountOutputs = m_multisignatureOutputs[in.amount];
      amountOutputs[in.outputIndex].isUsed = true;
    }
  }

  transaction.m_global_output_indexes.resize(transaction.tx.outputs.size());
  for (uint16_t output = 0; output < transaction.tx.outputs.size(); ++output) {
    if (transaction.tx.outputs[output].target.type() == typeid(key_output_t)) {
      auto& amountOutputs = m_outputs[transaction.tx.outputs[output].amount];
      transaction.m_global_output_indexes[output] = static_cast<uint32_t>(amountOutputs.size());
      amountOutputs.push_back(std::make_pair<>(transactionIndex, output));
    } else if (transaction.tx.outputs[output].target.type() == typeid(multi_signature_output_t)) {
      auto& amountOutputs = m_multisignatureOutputs[transaction.tx.outputs[output].amount];
      transaction.m_global_output_indexes[output] = static_cast<uint32_t>(amountOutputs.size());
      multisignature_output_usage_t outputUsage = { transactionIndex, output, false };
      amountOutputs.push_back(outputUsage);
    }
  }

  m_paymentIdIndex.add(transaction.tx);

  return true;
}

void Blockchain::popTransaction(const transaction_t& transaction, const crypto::hash_t& transactionHash) {
  transaction_index_t transactionIndex = m_transactionMap.at(transactionHash);
  for (size_t outputIndex = 0; outputIndex < transaction.outputs.size(); ++outputIndex) {
    const transaction_output_t& output = transaction.outputs[transaction.outputs.size() - 1 - outputIndex];
    if (output.target.type() == typeid(key_output_t)) {
      auto amountOutputs = m_outputs.find(output.amount);
      if (amountOutputs == m_outputs.end()) {
        logger(ERROR, BRIGHT_RED) <<
          "Blockchain consistency broken - cannot find specific amount in outputs map.";
        continue;
      }

      if (amountOutputs->second.empty()) {
        logger(ERROR, BRIGHT_RED) <<
          "Blockchain consistency broken - output array for specific amount is empty.";
        continue;
      }

      if (amountOutputs->second.back().first.block != transactionIndex.block || amountOutputs->second.back().first.transaction != transactionIndex.transaction) {
        logger(ERROR, BRIGHT_RED) <<
          "Blockchain consistency broken - invalid transaction index.";
        continue;
      }

      if (amountOutputs->second.back().second != transaction.outputs.size() - 1 - outputIndex) {
        logger(ERROR, BRIGHT_RED) <<
          "Blockchain consistency broken - invalid output index.";
        continue;
      }

      amountOutputs->second.pop_back();
      if (amountOutputs->second.empty()) {
        m_outputs.erase(amountOutputs);
      }
    } else if (output.target.type() == typeid(multi_signature_output_t)) {
      auto amountOutputs = m_multisignatureOutputs.find(output.amount);
      if (amountOutputs == m_multisignatureOutputs.end()) {
        logger(ERROR, BRIGHT_RED) <<
          "Blockchain consistency broken - cannot find specific amount in outputs map.";
        continue;
      }

      if (amountOutputs->second.empty()) {
        logger(ERROR, BRIGHT_RED) <<
          "Blockchain consistency broken - output array for specific amount is empty.";
        continue;
      }

      if (amountOutputs->second.back().isUsed) {
        logger(ERROR, BRIGHT_RED) <<
          "Blockchain consistency broken - attempting to remove used output.";
        continue;
      }

      if (amountOutputs->second.back().transactionIndex.block != transactionIndex.block || amountOutputs->second.back().transactionIndex.transaction != transactionIndex.transaction) {
        logger(ERROR, BRIGHT_RED) <<
          "Blockchain consistency broken - invalid transaction index.";
        continue;
      }

      if (amountOutputs->second.back().outputIndex != transaction.outputs.size() - 1 - outputIndex) {
        logger(ERROR, BRIGHT_RED) <<
          "Blockchain consistency broken - invalid output index.";
        continue;
      }

      amountOutputs->second.pop_back();
      if (amountOutputs->second.empty()) {
        m_multisignatureOutputs.erase(amountOutputs);
      }
    }
  }

  for (auto& input : transaction.inputs) {
    if (input.type() == typeid(key_input_t)) {
      size_t count = m_spent_keys.erase(::boost::get<key_input_t>(input).keyImage);
      if (count != 1) {
        logger(ERROR, BRIGHT_RED) <<
          "Blockchain consistency broken - cannot find spent key.";
      }
    } else if (input.type() == typeid(multi_signature_input_t)) {
      const multi_signature_input_t& in = ::boost::get<multi_signature_input_t>(input);
      auto& amountOutputs = m_multisignatureOutputs[in.amount];
      if (!amountOutputs[in.outputIndex].isUsed) {
        logger(ERROR, BRIGHT_RED) <<
          "Blockchain consistency broken - multisignature output not marked as used.";
      }

      amountOutputs[in.outputIndex].isUsed = false;
    }
  }

  m_paymentIdIndex.remove(transaction);

  size_t count = m_transactionMap.erase(transactionHash);
  if (count != 1) {
    logger(ERROR, BRIGHT_RED) <<
      "Blockchain consistency broken - cannot find transaction by hash.";
  }
}

void Blockchain::popTransactions(const block_entry_t& block, const crypto::hash_t& minerTransactionHash) {
  for (size_t i = 0; i < block.transactions.size() - 1; ++i) {
    popTransaction(block.transactions[block.transactions.size() - 1 - i].tx, block.bl.transactionHashes[block.transactions.size() - 2 - i]);
  }

  popTransaction(block.bl.baseTransaction, minerTransactionHash);
}

bool Blockchain::validateInput(const multi_signature_input_t& input, const crypto::hash_t& transactionHash, const crypto::hash_t& transactionPrefixHash, const std::vector<crypto::signature_t>& transactionSignatures) {
  assert(input.signatureCount == transactionSignatures.size());
  multisignature_outputs_container_t::const_iterator amountOutputs = m_multisignatureOutputs.find(input.amount);
  if (amountOutputs == m_multisignatureOutputs.end()) {
    logger(DEBUGGING) <<
      "transaction_t << " << transactionHash << " contains multisignature input with invalid amount.";
    return false;
  }

  if (input.outputIndex >= amountOutputs->second.size()) {
    logger(DEBUGGING) <<
      "transaction_t << " << transactionHash << " contains multisignature input with invalid outputIndex.";
    return false;
  }

  const multisignature_output_usage_t& outputIndex = amountOutputs->second[input.outputIndex];
  if (outputIndex.isUsed) {
    logger(DEBUGGING) <<
      "transaction_t << " << transactionHash << " contains double spending multisignature input.";
    return false;
  }

  const transaction_t& outputTransaction = m_blocks[outputIndex.transactionIndex.block].transactions[outputIndex.transactionIndex.transaction].tx;
  if (!is_tx_spendtime_unlocked(outputTransaction.unlockTime)) {
    logger(DEBUGGING) <<
      "transaction_t << " << transactionHash << " contains multisignature input which points to a locked transaction.";
    return false;
  }

  assert(outputTransaction.outputs[outputIndex.outputIndex].amount == input.amount);
  assert(outputTransaction.outputs[outputIndex.outputIndex].target.type() == typeid(multi_signature_output_t));
  const multi_signature_output_t& output = ::boost::get<multi_signature_output_t>(outputTransaction.outputs[outputIndex.outputIndex].target);
  if (input.signatureCount != output.requiredSignatureCount) {
    logger(DEBUGGING) <<
      "transaction_t << " << transactionHash << " contains multisignature input with invalid signature count.";
    return false;
  }

  size_t inputSignatureIndex = 0;
  size_t outputKeyIndex = 0;
  while (inputSignatureIndex < input.signatureCount) {
    if (outputKeyIndex == output.keys.size()) {
      logger(DEBUGGING) <<
        "transaction_t << " << transactionHash << " contains multisignature input with invalid signatures.";
      return false;
    }

    if (crypto::check_signature(transactionPrefixHash, output.keys[outputKeyIndex], transactionSignatures[inputSignatureIndex])) {
      ++inputSignatureIndex;
    }

    ++outputKeyIndex;
  }

  return true;
}

bool Blockchain::getLowerBound(uint64_t timestamp, uint64_t startOffset, uint32_t& height) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  assert(startOffset < m_blocks.size());

  auto bound = std::lower_bound(m_blocks.begin() + startOffset, m_blocks.end(), timestamp - m_currency.blockFutureTimeLimit(),
    [](const block_entry_t& b, uint64_t timestamp) { return b.bl.timestamp < timestamp; });

  if (bound == m_blocks.end()) {
    return false;
  }

  height = static_cast<uint32_t>(std::distance(m_blocks.begin(), bound));
  return true;
}

std::vector<crypto::hash_t> Blockchain::getBlockIds(uint32_t startHeight, uint32_t maxCount) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  return m_blockIndex.getBlockIds(startHeight, maxCount);
}

bool Blockchain::getBlockContainingTransaction(const crypto::hash_t& txId, crypto::hash_t& blockId, uint32_t& blockHeight) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  auto it = m_transactionMap.find(txId);
  if (it == m_transactionMap.end()) {
    return false;
  } else {
    blockHeight = m_blocks[it->second.block].height;
    blockId = getBlockIdByHeight(blockHeight);
    return true;
  }
}

bool Blockchain::getAlreadyGeneratedCoins(const crypto::hash_t& hash, uint64_t& generatedCoins) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  // try to find block in main chain
  uint32_t height = 0;
  if (m_blockIndex.getBlockHeight(hash, height)) {
    generatedCoins = m_blocks[height].already_generated_coins;
    return true;
  }

  // try to find block in alternative chain
  auto blockByHashIterator = m_alternative_chains.find(hash);
  if (blockByHashIterator != m_alternative_chains.end()) {
    generatedCoins = blockByHashIterator->second.already_generated_coins;
    return true;
  }

  logger(DEBUGGING) << "Can't find block with hash " << hash << " to get already generated coins.";
  return false;
}

bool Blockchain::getBlockSize(const crypto::hash_t& hash, size_t& size) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  // try to find block in main chain
  uint32_t height = 0;
  if (m_blockIndex.getBlockHeight(hash, height)) {
    size = m_blocks[height].block_cumulative_size;
    return true;
  }

  // try to find block in alternative chain
  auto blockByHashIterator = m_alternative_chains.find(hash);
  if (blockByHashIterator != m_alternative_chains.end()) {
    size = blockByHashIterator->second.block_cumulative_size;
    return true;
  }

  logger(DEBUGGING) << "Can't find block with hash " << hash << " to get block size.";
  return false;
}

bool Blockchain::getMultisigOutputReference(const multi_signature_input_t& txInMultisig, std::pair<crypto::hash_t, size_t>& outputReference) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  multisignature_outputs_container_t::const_iterator amountIter = m_multisignatureOutputs.find(txInMultisig.amount);
  if (amountIter == m_multisignatureOutputs.end()) {
    logger(DEBUGGING) << "transaction_t contains multisignature input with invalid amount.";
    return false;
  }
  if (amountIter->second.size() <= txInMultisig.outputIndex) {
    logger(DEBUGGING) << "transaction_t contains multisignature input with invalid outputIndex.";
    return false;
  }
  const multisignature_output_usage_t& outputIndex = amountIter->second[txInMultisig.outputIndex];
  const transaction_t& outputTransaction = m_blocks[outputIndex.transactionIndex.block].transactions[outputIndex.transactionIndex.transaction].tx;
  outputReference.first = getObjectHash(outputTransaction);
  outputReference.second = outputIndex.outputIndex;
  return true;
}

bool Blockchain::storeBlockchainIndices() {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  logger(INFO, BRIGHT_WHITE) << "Saving blockchain indices...";
  BlockchainIndicesSerializer ser(*this, getTailId(), logger.getLogger());

  if (!storeToBinaryFile(ser, m_currency.blockchainIndexesFileName())) {
    logger(ERROR, BRIGHT_RED) << "Failed to save blockchain indices";
    return false;
  }

  return true;
}

bool Blockchain::loadBlockchainIndices() {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);

  logger(INFO, BRIGHT_WHITE) << "Loading blockchain indices for BlockchainExplorer...";
  BlockchainIndicesSerializer loader(*this, Block::getHash(m_blocks.back().bl), logger.getLogger());

  loadFromBinaryFile(loader, m_currency.blockchainIndexesFileName());

  if (!loader.loaded()) {
    logger(WARNING, BRIGHT_YELLOW) << "No actual blockchain indices for BlockchainExplorer found, rebuilding...";
    std::chrono::steady_clock::time_point timePoint = std::chrono::steady_clock::now();

    m_paymentIdIndex.clear();
    m_timestampIndex.clear();
    m_generatedTransactionsIndex.clear();

    for (uint32_t b = 0; b < m_blocks.size(); ++b) {
      if (b % 1000 == 0) {
        logger(INFO, BRIGHT_WHITE) << "Height " << b << " of " << m_blocks.size();
      }
      const block_entry_t& block = m_blocks[b];
      m_timestampIndex.add(block.bl.timestamp, Block::getHash(block.bl));
      m_generatedTransactionsIndex.add(block.bl);
      for (uint16_t t = 0; t < block.transactions.size(); ++t) {
        const transaction_entry_t& transaction = block.transactions[t];
        m_paymentIdIndex.add(transaction.tx);
      }
    }

    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - timePoint;
    logger(INFO, BRIGHT_WHITE) << "Rebuilding blockchain indices took: " << duration.count();
  }
  return true;
}

bool Blockchain::getGeneratedTransactionsNumber(uint32_t height, uint64_t& generatedTransactions) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  return m_generatedTransactionsIndex.find(height, generatedTransactions);
}

bool Blockchain::getOrphanBlockIdsByHeight(uint32_t height, std::vector<crypto::hash_t>& blockHashes) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  return m_orthanBlocksIndex.find(height, blockHashes);
}

bool Blockchain::getBlockIdsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<crypto::hash_t>& hashes, uint32_t& blocksNumberWithinTimestamps) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  return m_timestampIndex.find(timestampBegin, timestampEnd, blocksNumberLimit, hashes, blocksNumberWithinTimestamps);
}

bool Blockchain::getTransactionIdsByPaymentId(const crypto::hash_t& paymentId, std::vector<crypto::hash_t>& transactionHashes) {
  std::lock_guard<decltype(m_blockchain_lock)> lk(m_blockchain_lock);
  return m_paymentIdIndex.find(paymentId, transactionHashes);
}

bool Blockchain::loadTransactions(const block_t& block, std::vector<transaction_t>& transactions) {
  transactions.resize(block.transactionHashes.size());
  size_t transactionSize;
  uint64_t fee;
  for (size_t i = 0; i < block.transactionHashes.size(); ++i) {
    if (!m_tx_pool.take_tx(block.transactionHashes[i], transactions[i], transactionSize, fee)) {
      tx_verification_context_t context;
      for (size_t j = 0; j < i; ++j) {
        if (!m_tx_pool.add_tx(transactions[i - 1 - j], context, true)) {
          throw std::runtime_error("Blockchain::loadTransactions, failed to add transaction to pool");
        }
      }

      return false;
    }
  }

  return true;
}

void Blockchain::saveTransactions(const std::vector<transaction_t>& transactions) {
  tx_verification_context_t context;
  for (size_t i = 0; i < transactions.size(); ++i) {
    if (!m_tx_pool.add_tx(transactions[transactions.size() - 1 - i], context, true)) {
      throw std::runtime_error("Blockchain::saveTransactions, failed to add transaction to pool");
    }
  }
}

bool Blockchain::addMessageQueue(MessageQueue<BlockchainMessage>& messageQueue) {
  return m_messageQueueList.insert(messageQueue);
}

bool Blockchain::removeMessageQueue(MessageQueue<BlockchainMessage>& messageQueue) {
  return m_messageQueueList.remove(messageQueue);
}

void Blockchain::sendMessage(const BlockchainMessage& message) {
  for (IntrusiveLinkedList<MessageQueue<BlockchainMessage>>::iterator iter = m_messageQueueList.begin(); iter != m_messageQueueList.end(); ++iter) {
    iter->push(message);
  }
}

bool Blockchain::isBlockInMainChain(const crypto::hash_t& blockId) {
  return m_blockIndex.hasBlock(blockId);
}

}
