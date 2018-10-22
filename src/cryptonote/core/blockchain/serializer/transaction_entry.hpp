
#pragma once

#include "cryptonote/structures/transaction.h"
#include <Serialization/ISerializer.h>

namespace cryptonote
{
struct TransactionEntry
{
    Transaction tx;
    std::vector<uint32_t> m_global_output_indexes;

    void serialize(ISerializer &s)
    {
        s(tx, "tx");
        s(m_global_output_indexes, "indexes");
    }
};
} // namespace cryptonote