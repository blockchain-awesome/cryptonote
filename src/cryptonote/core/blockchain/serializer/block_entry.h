#pragma once
#include <serialization/ISerializer.h>
#include "cryptonote/types.h"

namespace cryptonote
{
    void serialize(block_entry_t &be, ISerializer &s);
}
