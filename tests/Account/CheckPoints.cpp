// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <fstream>
#include "stream/StdInputStream.h"
#include "stream/StdOutputStream.h"
#include "serialization/BinaryInputStreamSerializer.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include "cryptonote/core/Checkpoints.h"
#include <logging/LoggerRef.h>
#include <logging/LoggerManager.h>

using namespace cryptonote;
using namespace Common;
using namespace Logging;

namespace
{

class CheckPointsTest : public ::testing::Test
{
public:
  CheckPointsTest() : ::testing::Test() {}
};

LoggerManager logManager;
LoggerRef logger(logManager, "checkpoint");

Checkpoints *cp = new Checkpoints(logManager);

TEST_F(CheckPointsTest, add)
{

  ASSERT_TRUE(cp->add(1, "84b6345731e2702cdaadc6ce5e5238c4ca5ecf48e3447136b2ed829b8a95f3ad"));
  ASSERT_TRUE(!cp->add(1, "84b634a731e2702cdaadc6ce5e5238c4ca5ecf48e3447136b2ed829b8a95f3ad"));
  ASSERT_TRUE(!cp->add(1, "84b634Q731e2702cdaadc6ce5e5238c4ca5ecf48e3447136b2ed829b8a95f3ad"));
  ASSERT_TRUE(cp->add(2, "84b6315731e2702cdaadc6ce5e5238c4ca5ecf48e3447136b2ed829b8a95f3ad"));
}

TEST_F(CheckPointsTest, isCheckpoint)
{
  ASSERT_TRUE(cp->isCheckpoint(1));
  ASSERT_TRUE(cp->isCheckpoint(2));
  ASSERT_TRUE(!cp->isCheckpoint(100));
}

TEST_F(CheckPointsTest, check)
{
  crypto::Hash h = NULL_HASH;
  ASSERT_TRUE(podFromHex("84b6345731e2702cdaadc6ce5e5238c4ca5ecf48e3447136b2ed829b8a95f3ad", h));
  ASSERT_TRUE(cp->check(1, h));
  ASSERT_TRUE(!cp->check(2, h));
  ASSERT_TRUE(podFromHex("84b6315731e2702cdaadc6ce5e5238c4ca5ecf48e3447136b2ed829b8a95f3ad", h));
  ASSERT_TRUE(cp->check(2, h));
  ASSERT_TRUE(!cp->check(1, h));
}

TEST_F(CheckPointsTest, isAllowed)
{
  ASSERT_TRUE(!cp->isAllowed(10, 0));
  ASSERT_TRUE(!cp->isAllowed(0, 0));
  ASSERT_TRUE(!cp->isAllowed(1, 1));
  ASSERT_TRUE(cp->isAllowed(1, 2));
  ASSERT_TRUE(!cp->isAllowed(2, 2));
  ASSERT_TRUE(cp->isAllowed(1, 3));
  ASSERT_TRUE(cp->isAllowed(3, 4));
  ASSERT_TRUE(cp->isAllowed(4, 5));
  ASSERT_TRUE(cp->isAllowed(6, 5));
}

} // namespace
