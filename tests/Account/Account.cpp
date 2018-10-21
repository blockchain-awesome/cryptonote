// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <fstream>
#include "stream/StdInputStream.h"
#include "Serialization/BinaryInputStreamSerializer.h"
#include "cryptonote/core/Account.h"

using namespace cryptonote;
using namespace Common;

namespace
{

class AccountTest : public ::testing::Test
{
public:
  AccountTest() : ::testing::Test() {}
};

TEST_F(AccountTest, create)
{
  AccountBase *acc = NULL;
  EXPECT_EQ(NULL, acc);
  acc = new AccountBase();
  EXPECT_EQ(true, !!acc);
}

TEST_F(AccountTest, generate)
{
  AccountBase *acc = new AccountBase();
  acc->generate();
  uint64_t time = acc->get_createtime();

  EXPECT_EQ(true, !!time);
}

TEST_F(AccountTest, serialize)
{
  AccountBase *acc = new AccountBase();
  acc->generate();

  std::string filename = "temp.txt";
  std::ifstream stdStream(filename, std::ios::binary);
  if (!stdStream)
  {
    return;
  }

  StdInputStream stream(stdStream);
  BinaryInputStreamSerializer s(stream);
  AccountKeys keys = acc->getAccountKeys();
  acc->serialize(s);
  ASSERT_TRUE(sizeof(keys) > 0);
  ASSERT_TRUE(sizeof(keys.address.spendPublicKey) == 32);
  ASSERT_TRUE(sizeof(keys.address.viewPublicKey) == 32);
  ASSERT_TRUE(sizeof(keys.spendSecretKey) == 32);
  ASSERT_TRUE(sizeof(keys.viewSecretKey) == 32);
  stdStream.close();
  std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
  int size = 0;

  if (file.is_open())
  {
    int size = file.tellg();
  }

  ASSERT_TRUE(size != 0);
}
} // namespace
