// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "serialization/ISerializer.h"
#include "cryptonote/types.h"

namespace cryptonote
{
  struct csi_t : public core_state_info_t
  {
    void serialize(ISerializer& s);
  };
}
