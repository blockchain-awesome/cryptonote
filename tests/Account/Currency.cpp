// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <fstream>
#include "stream/StdInputStream.h"
#include "stream/StdOutputStream.h"
#include "serialization/BinaryInputStreamSerializer.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include "cryptonote/core/Currency.h"
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
  cryptonote::CurrencyBuilder currencyBuilder(logManager, "./data");

  Currency c = currencyBuilder.currency();

  Block b = c.genesisBlock();

  std::string filename = c.txPoolFileName(true);
  std::string fullname = c.txPoolFileName();

  std::cout << "genesis block" << std::endl;
  std::cout << b.timestamp << std::endl;
  std::cout << b.majorVersion << std::endl;
  std::cout << b.minorVersion << std::endl;
  ASSERT_TRUE(filename == parameters::CRYPTONOTE_POOLDATA_FILENAME);
  ASSERT_TRUE(fullname != parameters::CRYPTONOTE_POOLDATA_FILENAME);
}
} // namespace
