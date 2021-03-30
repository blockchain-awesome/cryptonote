// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <fstream>
#include "stream/writer.h"
#include "serialization/BinaryInputStreamSerializer.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include "cryptonote/core/currency.h"
#include <logging/LoggerManager.h>

using namespace cryptonote;
using namespace Common;
using namespace Logging;

namespace
{

class CurrencyTest : public ::testing::Test
{
public:
  CurrencyTest() : ::testing::Test() {}
};

TEST_F(CurrencyTest, create)
{
  LoggerManager logManager;
  cryptonote::CurrencyBuilder currencyBuilder("./data", config::testnet::data, logManager);

  Currency c = currencyBuilder.currency();

  transaction_t tx = currencyBuilder.generateGenesisTransaction();

  block_t b = c.genesisBlock();

  std::string filename = c.txPoolFileName(true);
  std::string fullname = c.txPoolFileName();

  std::cout << "genesis block" << std::endl;
  std::cout << b.timestamp << std::endl;
  std::cout << b.majorVersion << std::endl;
  std::cout << b.minorVersion << std::endl;
  std::cout << tx.inputs.size() << std::endl;
  ASSERT_TRUE(filename == config::get().filenames.pool);
  ASSERT_TRUE(fullname != config::get().filenames.pool);
  ASSERT_TRUE(tx.version > 0);
  ASSERT_TRUE(tx.inputs.size() == 1);
  ASSERT_TRUE(tx.outputs.size() == 1);
  ASSERT_TRUE(boost::filesystem::exists(c.blockchainIndexesFileName()));
  ASSERT_TRUE(!boost::filesystem::exists(c.blockIndexesFileName()));
  ASSERT_TRUE(!boost::filesystem::exists(c.blocksCacheFileName()));
  ASSERT_TRUE(!boost::filesystem::exists(c.blocksFileName()));
}
} // namespace
