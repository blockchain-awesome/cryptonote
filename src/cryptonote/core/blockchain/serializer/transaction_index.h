#pragma once

#include <Serialization/ISerializer.h>

namespace cryptonote
{
struct TransactionIndex
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