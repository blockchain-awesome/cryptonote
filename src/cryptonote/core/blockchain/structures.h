#pragma once

#include "cryptonote/core/key.h"

namespace cryptonote
{
struct block_info_t
{
    uint32_t height;
    crypto::hash_t id;

    block_info_t()
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