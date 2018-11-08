#pragma once

#include <serialization/ISerializer.h>

namespace cryptonote
{
struct transaction_index_t
{
    uint32_t block;
    uint16_t transaction;

    void serialize(ISerializer &s)
    {
        s(block, "block");
        s(transaction, "tx");
    }
};

} // namespace cryptonote