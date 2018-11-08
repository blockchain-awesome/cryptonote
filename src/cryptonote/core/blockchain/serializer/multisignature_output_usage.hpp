#pragma once

#include "transaction_index.h"

namespace cryptonote
{
struct MultisignatureOutputUsage
{
    transaction_index_t transactionIndex;
    uint16_t outputIndex;
    bool isUsed;

    void serialize(ISerializer &s)
    {
        s(transactionIndex, "txindex");
        s(outputIndex, "outindex");
        s(isUsed, "used");
    }
};

} // namespace cryptonote