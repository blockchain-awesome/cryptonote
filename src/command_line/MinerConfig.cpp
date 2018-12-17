// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MinerConfig.h"

#include "command_line/options.h"

using namespace command_line;

namespace cryptonote
{

MinerConfig::MinerConfig()
{
  miningThreads = 0;
}

void MinerConfig::initOptions(boost::program_options::options_description &desc)
{
  command_line::init();
  add_arg(desc, arg_extra_messages);
  add_arg(desc, arg_start_mining);
  add_arg(desc, arg_mining_threads);
}

void MinerConfig::init(const boost::program_options::variables_map &options)
{
  if (has_arg(options, arg_extra_messages))
  {
    extraMessages = get_arg(options, arg_extra_messages);
  }

  if (has_arg(options, arg_start_mining))
  {
    startMining = get_arg(options, arg_start_mining);
  }

  if (has_arg(options, arg_mining_threads))
  {
    miningThreads = get_arg(options, arg_mining_threads);
  }
}

} //namespace cryptonote
