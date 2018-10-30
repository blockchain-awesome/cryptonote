// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "options.h"

namespace command_line
{

namespace po = boost::program_options;
class Daemon: public CommandParser
{
public:
Daemon(OptionsNames& names): CommandParser(names) {};
virtual bool innerParse();

void init();
bool checkVersion();
void printGenesisTx();

bool parseHelp();
void parseConfigFile();
};

} // namespace command_line
