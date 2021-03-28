#pragma once

#include "crypto/types.h"
#include "serialize.h"

namespace serialize
{

  namespace complex
  {
    std::istream &operator>>(std::istream &i, std::string &v);
    std::ostream &operator<<(std::ostream &o, std::string &v);
  }

}