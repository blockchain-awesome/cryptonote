// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <fstream>
#include <cstring>

#include "crypto/crypto.h"
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

  crypto::hash_t hash = ba.getHash();
  std::string hex = ba.toString();

  binary_array_t bat1;
  
  cryptonote::BinaryArray::to(bat, bat1);
  // cryptonote::BinaryArray::to(add);
  cryptonote::BinaryArray ba1(bat1);

  ASSERT_TRUE(ba.toString() == "hello");
  // ASSERT_TRUE(ba1.toString() == "hello");
  ASSERT_TRUE(hex.length() > 0);
  ASSERT_TRUE(memcmp(&hash, &cryptonote::NULL_HASH, sizeof (crypto::hash_t)) != 0);

}

} // namespace
