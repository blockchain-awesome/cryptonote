// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <fstream>
#include <cstring>

#include "cryptonote/crypto/crypto.h"
#include "cryptonote/structures/array.h"
#include "cryptonote/core/key.h"

namespace
{

class BinaryArrayTest : public ::testing::Test
{
public:
  BinaryArrayTest() : ::testing::Test() {}
};

TEST_F(BinaryArrayTest, create)
{
  binary_array_t bat;
  // cryptonote::transaction_t add;
  bat.push_back('h');
  bat.push_back('e');
  bat.push_back('l');
  bat.push_back('l');
  bat.push_back('o');
  cryptonote::BinaryArray ba(bat);

  hash_t hash = ba.hash();
  std::string hex = ba.toString();

  binary_array_t bat1;

  cryptonote::BinaryArray::to(bat, bat1);
  // cryptonote::BinaryArray::to(add);
  cryptonote::BinaryArray ba1(bat1);

  ASSERT_TRUE(ba.toString() == "hello");
  // ASSERT_TRUE(ba1.toString() == "hello");
  ASSERT_TRUE(hex.length() > 0);
  ASSERT_TRUE(memcmp(&hash, &cryptonote::NULL_HASH, sizeof(hash_t)) != 0);

  ASSERT_TRUE(IBinary::to(bat).compare("hello") == 0);

  char a[] = "hello world";
  std::string out = IBinary::to(a, sizeof(a));
  std::cout << "out:" << out << std::endl;
  std::cout << "length:" << out.length() << std::endl;
  ASSERT_TRUE(out.compare(std::string("hello world", sizeof(a))) == 0);
}

} // namespace
