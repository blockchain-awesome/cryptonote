#pragma once

#include "crypto/types.h"
#include "reader.h"
#include "writer.h"

#include "cryptonote.h"
#include "crypto.h"
#include "cryptonote/types.h"

using namespace cryptonote;

namespace stream
{
  namespace cryptonote
  {
    Reader &operator>>(Reader &i, block_header_t &v);
    Reader &operator>>(Reader &i, block_t &v);

    Writer &operator<<(Writer &o, const block_header_t &v);
    Writer &operator<<(Writer &o, const block_t &v);
  }
}