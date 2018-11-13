#include "block.h"
#include "array.h"
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

bool Block::getHash(const block_t& block, crypto::hash_t& hash) {
  binary_array_t ba;
  if (!get_block_hashing_blob(block, ba)) {
    return false;
  }

  return getObjectHash(ba, hash);
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
      Common::fromHex(genesisCoinbaseTxHex, minerTxBlob) &&
      fromBinaryArray(block.baseTransaction, minerTxBlob);

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

  get_block_longhash(block, proofOfWork);
  return check_hash(proofOfWork, currentDiffic);
}
} // namespace cryptonote