// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "common.h"

namespace command_line
{

namespace po = boost::program_options;
extern const arg_descriptor<bool> arg_print_genesis_tx;
extern const arg_descriptor<std::string> arg_data_dir;
extern const arg_descriptor<std::string> arg_config_file;

class Daemon: public CommandParser
{
public:
Daemon(OptionsNames& names): CommandParser(names) {};
virtual bool innerParse();

bool parseHelp();
};

} // namespace command_line
