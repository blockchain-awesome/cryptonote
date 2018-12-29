// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"
#include "common/math.hpp"
#include <vector>

namespace
{

class MathTest : public ::testing::Test
{
public:
  MathTest() : ::testing::Test() {}
};

TEST_F(MathTest, get)
{
  std::vector<int> v;
  math::medianValue(v);
  v.push_back(1);
  ASSERT_TRUE(math::medianValue(v) == 1);
  v.push_back(3);
  v.push_back(-1);
  v.push_back(4);
  v.push_back(-5);
  v.push_back(6);
  ASSERT_TRUE(math::medianValue(v) == 2);
  v.push_back(7);
  ASSERT_TRUE(math::medianValue(v) == 3);


}

} // namespace
