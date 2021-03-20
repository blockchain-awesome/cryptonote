// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include "reader.h"

namespace Common
{

  class StringInputStream : public Reader
  {
  public:
    StringInputStream(const std::string &in);

  private:
    size_t offset;
  };

}
