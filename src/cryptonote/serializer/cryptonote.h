

#pragma once

#include "serialize.h"

#include "cryptonote/types.h"

namespace serialize
{
  namespace cryptonote
  {

    std::istream &operator>>(std::istream &i, transaction_index_t &v);
    std::istream &operator>>(std::istream &i, transaction_map_t &v);
    
    std::ostream &operator<<(std::ostream &o, const transaction_index_t &v);
    std::ostream &operator<<(std::ostream &o, const transaction_map_t &v);
  }
}