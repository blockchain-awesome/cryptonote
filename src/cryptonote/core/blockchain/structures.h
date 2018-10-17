#pragma once

#include "cryptonote/core/CryptoNoteBasic.h"

namespace cryptonote
{
struct BlockInfo
{
    uint32_t height;
    crypto::Hash id;

    BlockInfo()
    {
        clear();
    }

    void clear()
    {
        height = 0;
        id = cryptonote::NULL_HASH;
    }

    bool empty() const
    {
        return id == cryptonote::NULL_HASH;
    }
};
} // namespace cryptonote