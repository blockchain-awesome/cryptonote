#pragma once

#include <cstdint>
#include <unordered_map>
#include "crypto/types.h"

struct transaction_index_t
{
    uint32_t block;
    uint16_t transaction;
};

typedef uint64_t difficulty_t;

typedef std::unordered_map<hash_t, transaction_index_t> transaction_map_t;
