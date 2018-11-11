#include "block.h"
#include "array.h"
#include <boost/utility/value_init.hpp>
#include "config/common.h"
#include "common/StringTools.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include <boost/system/system_error.hpp>

namespace cryptonote
{

block_t Block::genesis(config::config_t& conf)
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
} // namespace cryptonote