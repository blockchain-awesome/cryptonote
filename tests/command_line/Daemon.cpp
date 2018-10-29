// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include "command_line/daemon.h"
#include "parameters.h"

using namespace command_line;

namespace
{

class DaemonTest : public ::testing::Test
{
public:
  DaemonTest() : ::testing::Test() {}
};

TEST_F(DaemonTest, run)
{
  OptionsNames names;
  names.command = "Command line options";
  names.setting = "Command line options and settings options";
  names.full = "Allowed options";
  Daemon cli(names);
  cli.init();
  // bool r = cli.parse(my_argc, my_argv, [] {
  //   return true;
  // });
  // EXPECT_EQ(false, r);
}

} // namespace
