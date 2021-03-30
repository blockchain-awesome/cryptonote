

#pragma once

#include "serialize.h"

#include "cryptonote/types.h"

namespace serialize
{
  namespace cryptonote
  {
    inline std::istream &operator>>(std::istream &i, const transaction_index_t &v) { return serialize(i, v); }
    inline std::ostream &operator<<(std::ostream &o, const transaction_index_t &v) { return serialize(o, v); }
  }
}