#pragma once

#include "cryptonote/core/transaction/transaction.h"

namespace cryptonote
{

struct block_header_t
{
    uint8_t majorVersion;
    uint8_t minorVersion;
    uint32_t nonce;
    uint64_t timestamp;
    crypto::Hash previousBlockHash;
};

struct block_t : public block_header_t
{
    Transaction baseTransaction;
    std::vector<crypto::Hash> transactionHashes;
};

} // namespace cryptonote