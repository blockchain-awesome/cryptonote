#pragma once

#include "config/common.h"
#include "cryptonote/core/transaction/transaction.h"
#include "cryptonote/core/difficulty.h"

namespace cryptonote
{

    struct block_header_t
    {
        uint8_t majorVersion;
        uint8_t minorVersion;
        uint32_t nonce;
        uint64_t timestamp;
        hash_t previousBlockHash;
    };

    struct block_t : public block_header_t
    {
        transaction_t baseTransaction;
        std::vector<hash_t> transactionHashes;
    };

} // namespace cryptonote