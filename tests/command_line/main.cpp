// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

int my_argc;
char** my_argv;

int main(int argc, char** argv)
{
  my_argc = argc,
  my_argv = argv;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
