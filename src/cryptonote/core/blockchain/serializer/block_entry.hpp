#pragma once

#include "transaction_entry.hpp"
#include "cryptonote/core/Difficulty.h"
#include "cryptonote/structures/block.h"
#include <serialization/ISerializer.h>

namespace cryptonote
{
struct block_entry_t
{
    block_t bl;
    uint32_t height;
    uint64_t block_cumulative_size;
    difficulty_t cumulative_difficulty;
    uint64_t already_generated_coins;
    std::vector<transaction_entry_t> transactions;

    void serialize(ISerializer &s)
    {
        s(bl, "block");
        s(height, "height");
        s(block_cumulative_size, "block_cumulative_size");
        s(cumulative_difficulty, "cumulative_difficulty");
        s(already_generated_coins, "already_generated_coins");
        s(transactions, "transactions");
    }
};
} // namespace cryptonote