// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>

#include "cryptonote_core/Currency.h"

namespace cryptonote {

bool validateAddress(const std::string& address, const cryptonote::Currency& currency);

}
