#pragma once

#include "crypto/types.h"
#include "serialize.h"

namespace stream
{
  namespace complex
  {
    vistream &operator>>(vistream &i, std::string &v);
    vostream &operator<<(vostream &o, std::string &v);
  }
}