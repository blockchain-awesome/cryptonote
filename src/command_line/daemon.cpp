// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/os.h"
#include "daemon.h"
#include "logging/ConsoleLogger.h"
#include "common/StringTools.h"
#include "version.h"
#include "cryptonote/core/Core.h"
#include "cryptonote/core/CryptoNoteTools.h"

namespace command_line
{
bool Daemon::innerParse()
{
    if (!parseHelp())
    {
        return false;
    }
    return true;
}

bool Daemon::parseHelp()
{
    if (command_line::get_arg(vm, arg_help))
    {
        std::cout << cryptonote::CRYPTONOTE_NAME << " v" << PROJECT_VERSION_LONG << ENDL << ENDL;
        std::cout << desc_options << std::endl;
        return false;
    }
    return true;
}

} // namespace command_line
