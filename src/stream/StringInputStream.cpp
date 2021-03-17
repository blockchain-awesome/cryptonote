// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "StringInputStream.h"
#include <string.h>

namespace Common {

StringInputStream::StringInputStream(const std::string& in) : Reader(in.c_str(), in.length()), offset(0) {
}

}
