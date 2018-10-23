#pragma once

#include "cryptonote/core/transaction/transaction.h"

namespace cryptonote
{

struct BlockHeader
{
    uint8_t majorVersion;
    uint8_t minorVersion;
    uint32_t nonce;
    uint64_t timestamp;
    crypto::Hash previousBlockHash;
};

struct Block : public BlockHeader
{
    Transaction baseTransaction;
    std::vector<crypto::Hash> transactionHashes;
};

} // namespace cryptonote