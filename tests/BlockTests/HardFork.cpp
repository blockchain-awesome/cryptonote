// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"
#include "cryptonote/core/hardfork.h"

using namespace cryptonote;
using namespace config;

class HardForkTest : public ::testing::Test
{
public:
  HardForkTest() : ::testing::Test() {}
};


TEST_F(HardForkTest, create)
{
  config::setType(config::TESTNET);
  HardFork * hf = new HardFork(config::get().hardforks);

  ASSERT_TRUE(hf->getMajorVersion(1) == 7);
  // ASSERT_TRUE(hf->getMajorVersion(99) == 1);
  ASSERT_TRUE(hf->getMajorVersion(2) == 7);
  ASSERT_TRUE(hf->getMajorVersion(3) == 7);
  ASSERT_TRUE(hf->getMajorVersion(4) == 7);
}
