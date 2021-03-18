// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <fstream>
#include "stream/reader.h"
#include "stream/StdOutputStream.h"
#include "serialization/BinaryInputStreamSerializer.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include "cryptonote/core/account.h"

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
  Account *acc = NULL;
  EXPECT_EQ(NULL, acc);
  acc = new Account();
  EXPECT_EQ(true, !!acc);
}

TEST_F(AccountTest, generate)
{
  Account *acc = new Account();
  acc->generate();
  uint64_t time = acc->getCreatetime();

  EXPECT_EQ(true, !!time);
}

TEST_F(AccountTest, serialize)
{
  Account *acc = new Account();
  acc->generate();

  std::string filename = "temp.txt";
  std::fstream fstream;
  fstream.open(filename, std::fstream::out);
  std::cerr << "Failed to open file : " << errno << std::endl;

  ASSERT_TRUE(!!fstream);

  StdOutputStream stream(fstream);
  BinaryOutputStreamSerializer s(stream);
  account_keys_t keys = acc->getAccountKeys();
  acc->serialize(s);
  ASSERT_TRUE(sizeof(keys) > 0);
  ASSERT_TRUE(sizeof(keys.address.spendPublicKey) == 32);
  ASSERT_TRUE(sizeof(keys.address.viewPublicKey) == 32);
  ASSERT_TRUE(sizeof(keys.spendSecretKey) == 32);
  ASSERT_TRUE(sizeof(keys.viewSecretKey) == 32);
  fstream.close();

  Account *acc1 = new Account();

  std::fstream of;
  of.open(filename, std::fstream::in);

  Reader inStream(&of);
  BinaryInputStreamSerializer inS(inStream);

  acc1->serialize(inS);
  account_keys_t keys1 = acc1->getAccountKeys();
  ASSERT_TRUE(0 == memcmp(&keys, &keys1, sizeof(account_keys_t)));
}
} // namespace
