#pragma once

#include <serialization/ISerializer.h>
#include "cryptonote/types.h"

namespace cryptonote
{
    void serialize(multisignature_output_usage_t &usage, ISerializer &s);
}
