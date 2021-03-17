// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <istream>
#include "reader.h"

namespace Common
{

  class StdInputStream : public Reader
  {
  public:
    StdInputStream(std::istream &in);
    StdInputStream &operator=(const StdInputStream &) = delete;
  };

}
