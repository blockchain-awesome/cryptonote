// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"
#include "common/filesystem.h"

TEST(PathTools, NativePathToGeneric) {

#ifdef _WIN32
  const std::string input = "C:\\Windows\\System\\etc\\file.exe";
  const std::string output = "C:/Windows/System/etc/file.exe";
#else
  const std::string input = "/var/tmp/file.tmp";
  const std::string output = input;

#endif

  auto path = std::filesystem::path::generic_string(input);
  ASSERT_EQ(output, path);
}
