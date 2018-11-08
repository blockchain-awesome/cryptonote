// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>
#include <vector>

#include "crypto/hash.h"

namespace cryptonote
{
    typedef std::uint64_t difficulty_t;

    bool check_hash(const crypto::hash_t &hash, difficulty_t difficulty);
}
