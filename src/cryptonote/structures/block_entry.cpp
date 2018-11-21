#include "common/varint.h"
#include "common/StringTools.h"
#include "block.h"
#include "array.hpp"
#include <boost/utility/value_init.hpp>
#include "config/common.h"
#include "common/StringTools.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include <boost/system/system_error.hpp>
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "block_entry.h"

namespace cryptonote
{

std::string Block::toString()
{
  return Block::toString(m_block);
}
std::string Block::toString(const block_entry_t &block)
{
  std::stringstream ss;

  ss << "height: " << block.height << std::endl;
  ss << "timestamp: " << block.bl.timestamp << std::endl;
  ss << "cumulative difficulty: " << block.cumulative_difficulty << std::endl;
  ss << "cumulative size: " << block.block_cumulative_size << std::endl;
  ss << "hash: " << Block::getHash(block.bl) << std::endl;
  ss << "nonce " << block.bl.nonce << std::endl;
  ss << "tx_count " << block.bl.transactionHashes.size() << std::endl;
  return ss.str();
}

bool Block::getBlob(const block_t& b, binary_array_t& ba) {
  if (!BinaryArray::to(static_cast<const block_header_t&>(b), ba)) {
    return false;
  }

  hash_t treeRootHash = get_tx_tree_hash(b);
  ba.insert(ba.end(), treeRootHash.data, treeRootHash.data + 32);
  auto transactionCount = array::fromString(varint::get(b.transactionHashes.size() + 1));
  ba.insert(ba.end(), transactionCount.begin(), transactionCount.end());
  return true;
}

bool Block::getLongHash(const block_t& b, crypto::hash_t& res) {
  binary_array_t bd;
  if (!Block::getBlob(b, bd)) {
    return false;
  }

  cn_slow_hash(bd.data(), bd.size(), res);
  return true;
}

bool Block::getHash(const block_t& block, crypto::hash_t& hash) {
  binary_array_t ba;
  if (!Block::getBlob(block, ba)) {
    return false;
  }

  return BinaryArray::objectHash(ba, hash);
}

hash_t Block::getHash(const block_t&block) {
  hash_t hash = NULL_HASH;
  Block::getHash(block, hash);
  return hash;
}
block_t Block::genesis(config::config_t &conf)
{
  block_t block = boost::value_initialized<block_t>();

  std::string genesisCoinbaseTxHex = conf.block.genesis_coinbase_tx_hex;
  binary_array_t minerTxBlob;

  bool r =
      hex::fromString(genesisCoinbaseTxHex, minerTxBlob) &&
      BinaryArray::from(block.baseTransaction, minerTxBlob);

  if (!r)
  {
    throw std::runtime_error("Failed to create genesis hex!");
  }

  block.majorVersion = conf.block.version.major;
  block.minorVersion = conf.block.version.miner;
  block.timestamp = 0;
  block.nonce = 70;
  return block;
}

bool Block::checkProofOfWork(const block_t &block, difficulty_t currentDiffic,
                             crypto::hash_t &proofOfWork)
{

  Block::getLongHash(block, proofOfWork);
  return check_hash(proofOfWork, currentDiffic);
}
} // namespace cryptonote