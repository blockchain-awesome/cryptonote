#pragma once
#include "block_entry.h"

namespace cryptonote
{
    void serialize(block_entry_t &be, ISerializer &s)
    {
        s(be.bl, "block");
        s(be.height, "height");
        s(be.block_cumulative_size, "block_cumulative_size");
        s(be.cumulative_difficulty, "cumulative_difficulty");
        s(be.already_generated_coins, "already_generated_coins");
        s(be.transactions, "transactions");
    }
}
