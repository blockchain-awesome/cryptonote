// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>

#include "crypto/types.h"

extern "C"
{

    typedef struct
    {
        uint8_t target;  // seconds
        uint8_t cut;     //  timestamps to cut after sorting
        uint16_t lag;    //
        uint32_t window; // expected numbers of blocks per day

    } difficulty_config_t;
}

namespace cryptonote
{

typedef std::uint64_t difficulty_t;

extern "C"
{

    bool check_hash(const hash_t *hash, difficulty_t difficulty);
    uint64_t next_difficulty(uint64_t *timestamps,
                             uint16_t timestamps_length,
                             uint64_t *cumulativeDifficulties,
                             uint16_t difficulties_length,
                             uint64_t *config);
}

} // namespace cryptonote
