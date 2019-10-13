// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>

#include "cryptonote/crypto/hash.h"

using namespace crypto;

namespace cryptonote
{
typedef std::uint64_t difficulty_t;

extern "C"
{
    bool check_hash(const hash_t *hash, difficulty_t difficulty);
}

} // namespace cryptonote
