// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <fstream>
#include <cstring>
#include <algorithm>
#include "common/hex.h"
#include "common/stream.h"

namespace
{

class StreamTest : public ::testing::Test
{
public:
  StreamTest() : ::testing::Test() {}
};

TEST_F(StreamTest, saveload)
{
  std::string filename = "stream.txt";
  std::string saveString = "hello world";
  std::string readString = "";
  stream::save(filename, saveString);
  stream::load(filename, readString);

  ASSERT_TRUE(readString.compare(saveString) == 0);
}

TEST_F(StreamTest, hex)
{
  uint8_t z = hex::fromString('0');
  uint8_t o = hex::fromString('1');
  uint8_t a = hex::fromString('a');
  uint8_t b = hex::fromString('b');
  uint8_t c = hex::fromString('A');
  ASSERT_TRUE(o == 1);
  ASSERT_TRUE(o - z == 1);
  ASSERT_TRUE(b - a == 1);
  ASSERT_TRUE(c == 10);
  ASSERT_ANY_THROW(hex::fromString('Z'));
  ASSERT_ANY_THROW(hex::fromString('z'));
  ASSERT_ANY_THROW(hex::fromString('@'));

  std::string hex = "13030393FACD34ADEFABCDEF";
  uint8_t *u = new uint8_t[hex.size() / 2];
  hex::fromString(hex, u, hex.size() / 2);
  ASSERT_TRUE(u[0] == 0x13);
  std::string convertedHex;
  hex::toString(u, hex.size() / 2, convertedHex);
  std::cout << "convertedHex:" << convertedHex << std::endl;

  std::transform(convertedHex.begin(), convertedHex.end(), convertedHex.begin(), ::toupper);
  ASSERT_TRUE(hex.compare(convertedHex) == 0);

  binary_array_t ba;

  for (int i = 0; i < hex.size() / 2; i++)
  {
    ba.push_back(u[i]);
  }
  std::string convertedHex1;
  hex::toString(ba, convertedHex1);
  std::transform(convertedHex1.begin(), convertedHex1.end(), convertedHex1.begin(), ::toupper);

  ASSERT_TRUE(hex.compare(convertedHex1) == 0);

  std::string hex1 = "13030393FACD34ADETFABCDEF";
  ASSERT_FALSE(hex::fromString(hex1, ba));
}

} // namespace
