
#pragma once

#include "crypto/types.h"
#include "reader.h"
#include "writer.h"

#include "cryptonote.h"
#include "crypto.h"
#include "cryptonote/core/transaction/types.h"

using namespace cryptonote;

namespace stream
{
  namespace cryptonote
  {
    Reader &operator>>(Reader &i,  base_input_t &v);
    Reader &operator>>(Reader &i,  key_input_t &v);
    // Reader &operator>>(Reader &i, transaction_prefix_t &v);

    Writer &operator<<(Writer &o, const base_input_t &v);
    Writer &operator<<(Writer &o, const key_input_t &v);
    // Writer &operator<<(Writer &o, const public_key_t &v);
  }
}