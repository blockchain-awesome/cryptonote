#pragma once

#include "cryptonote/core/blockchain/structures.h"

namespace cryptonote
{

namespace transaction
{

struct TransactionCheckInfo
{
    BlockInfo maxUsedBlock;
    BlockInfo lastFailedBlock;
};

struct TransactionDetails : public TransactionCheckInfo
{
    crypto::hash_t id;
    Transaction tx;
    size_t blobSize;
    uint64_t fee;
    bool keptByBlock;
    time_t receiveTime;
};

struct TransactionPriorityComparator
{
    // lhs > hrs
    bool operator()(const TransactionDetails &lhs, const TransactionDetails &rhs) const
    {
        // price(lhs) = lhs.fee / lhs.blobSize
        // price(lhs) > price(rhs) -->
        // lhs.fee / lhs.blobSize > rhs.fee / rhs.blobSize -->
        // lhs.fee * rhs.blobSize > rhs.fee * lhs.blobSize
        uint64_t lhs_hi, lhs_lo = mul128(lhs.fee, rhs.blobSize, &lhs_hi);
        uint64_t rhs_hi, rhs_lo = mul128(rhs.fee, lhs.blobSize, &rhs_hi);

        return
            // prefer more profitable transactions
            (lhs_hi > rhs_hi) ||
            (lhs_hi == rhs_hi && lhs_lo > rhs_lo) ||
            // prefer smaller
            (lhs_hi == rhs_hi && lhs_lo == rhs_lo && lhs.blobSize < rhs.blobSize) ||
            // prefer older
            (lhs_hi == rhs_hi && lhs_lo == rhs_lo && lhs.blobSize == rhs.blobSize && lhs.receiveTime < rhs.receiveTime);
    }
};
} // namespace transaction

} // namespace cryptonote