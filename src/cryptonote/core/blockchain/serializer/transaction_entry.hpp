
#pragma once

#include "cryptonote/core/transaction/transaction.h"
#include <serialization/ISerializer.h>

namespace cryptonote
{
struct TransactionEntry
{
    transaction_t tx;
    std::vector<uint32_t> m_global_output_indexes;

    void serialize(ISerializer &s)
    {
        s(tx, "tx");
        s(m_global_output_indexes, "indexes");
    }
};
} // namespace cryptonote