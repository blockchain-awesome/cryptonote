#pragma once

#include <serialization/ISerializer.h>

#include "cryptonote/types.h"

namespace cryptonote
{
    void serialize(transaction_index_t &idx, ISerializer &s);

} // namespace cryptonote