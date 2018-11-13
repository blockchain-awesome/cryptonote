// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <fstream>
#include "stream/StdInputStream.h"
#include "stream/StdOutputStream.h"
#include "serialization/BinaryInputStreamSerializer.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include <logging/LoggerManager.h>
#include "cryptonote/structures/block_entry.h"
#include "cryptonote/core/transaction/transaction.h"
#include "cryptonote/core/currency.h"
#include "cryptonote/core/blockchain/serializer/basics.h"
#include "config/common.h"

#include "cryptonote/core/CryptoNoteFormatUtils.h"

using namespace cryptonote;
using namespace config;
using namespace Common;
using namespace Logging;

namespace
{

class BlockTest : public ::testing::Test
{
public:
  BlockTest() : ::testing::Test() {}
};

const char GENESIS_COINBASE_TX_HEX[] = "011c01f00101029b2e4c0281c0b02e7c53291a94d1d0cbff8883f8024f5142ee494ffbbd0880712101398fb9ec4e76aeef124dfb779de715022efd619e63d7516f8b1470266f5da1fd";
const char P2P_STAT_TRUSTED_PUB_KEY[] = "8f80f9a5a434a9f1510d13336228debfee9c918ce505efe225d8c94d045fa115";

seeds_t seeds = {
    "69.171.73.252:19800",
    "39.108.160.252:19800",
    "144.202.10.183:19800",
};

checkpoints_t checkpoints = {};
config_t data = {
    {GENESIS_COINBASE_TX_HEX,
     {1, 0, 0}},
    {1, 0, 0},
    {P2P_STAT_TRUSTED_PUB_KEY,
     {1, 0, 0}},
    seeds,
    checkpoints};
}; // namespace

TEST_F(BlockTest, create)
{
  LoggerManager logManager;
  cryptonote::CurrencyBuilder currencyBuilder("./data", config::testnet::data, logManager);

  Currency c = currencyBuilder.currency();

  cryptonote::transaction_t tx = currencyBuilder.generateGenesisTransaction();

  block_t b = c.genesisBlock();
  block_t b1 = block_t();
  block_entry_t blockEntry = boost::value_initialized<block_entry_t>();
  blockEntry.bl = b;

  Block block = Block(blockEntry);
  std::string ss = block.toString();

  try
  {
    block_t b1 = Block::genesis(data);

    FAIL() << "Expected genesis error!";
  }
  catch (std::runtime_error const &err)
  {
    EXPECT_EQ(err.what(), std::string("Failed to create genesis hex!"));
  }
  catch (...)
  {
    FAIL() << "Failed to create genesis hex!";
  }

  hash_t h;
  hash_t h1 = boost::value_initialized<hash_t>();

  ASSERT_TRUE(Block::getLongHash(b, h));
  ASSERT_TRUE(Block::checkProofOfWork(b, 0, h));

  // std::string filename = c.txPoolFileName(true);
  // std::string fullname = c.txPoolFileName();

  // std::cout << "genesis block" << std::endl;
  // std::cout << b.timestamp << std::endl;
  // std::cout << b.majorVersion << std::endl;
  // std::cout << b.minorVersion << std::endl;
  // std::cout << tx.inputs.size() << std::endl;
  // ASSERT_TRUE(filename == parameters::CRYPTONOTE_POOLDATA_FILENAME);
  // ASSERT_TRUE(fullname != parameters::CRYPTONOTE_POOLDATA_FILENAME);
  // ASSERT_TRUE(tx.version > 0);
  ASSERT_TRUE(ss.length() > 1);
  // ASSERT_TRUE(tx.outputs.size() == 1);
  // ASSERT_TRUE(boost::filesystem::exists(c.blockchainIndexesFileName()));
  // ASSERT_TRUE(boost::filesystem::exists(c.blockIndexesFileName()));
  // ASSERT_TRUE(boost::filesystem::exists(c.blocksCacheFileName()));
  // ASSERT_TRUE(boost::filesystem::exists(c.blocksFileName()));

} // namespace
