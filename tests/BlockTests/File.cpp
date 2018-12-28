// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <fstream>

#include "common/file.h"

#ifdef __linux
#include <unistd.h>
#endif

namespace
{

class FileTest : public ::testing::Test
{
public:
  FileTest() : ::testing::Test() {}
};

TEST_F(FileTest, create)
{
  std::string filename = "/aaaa";
#ifdef __linux
  if (getuid())
  {
    //Should Not be root to executed this code
    ASSERT_FALSE(std::file::create(filename));
  }
#endif
  filename = "./aaa";
  ASSERT_TRUE(std::file::create(filename));
}
TEST_F(FileTest, ALL)
{
  std::string filename = "./temp.file";
  std::file::unlink(filename);
  ASSERT_FALSE(std::file::open(filename));
  std::cout << "failed " << std::endl;
  std::fstream fs = std::file::open("./temp.file", true);
  uint64_t count = 0;
  uint64_t outCount = 1000;
  std::file::write(filename, reinterpret_cast<char *>(&count), sizeof count);
  std::file::read(filename, reinterpret_cast<char *>(&outCount), sizeof count);
  ASSERT_TRUE(outCount == 0);
  std::cout << "reopened " << std::endl;
  std::file::unlink(filename);
}

} // namespace
