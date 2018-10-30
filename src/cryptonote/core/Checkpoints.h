// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include <map>
#include "common/StringTools.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "cryptonote/core/key.h"
#include "cryptonote/core/serialize.h"
#include <logging/LoggerRef.h>

namespace cryptonote
{
  class Checkpoints
  {
  public:
    Checkpoints(Logging::ILogger& log);

    bool add(uint32_t height, const std::string& hash_str);
    bool isCheckpoint(uint32_t height) const;
    bool check(uint32_t height, const crypto::Hash& h) const;
    bool check(uint32_t height, const crypto::Hash& h, bool& is_a_checkpoint) const;
    bool isAllowed(uint32_t blockchain_height, uint32_t block_height) const;

  private:
    std::map<uint32_t, crypto::Hash> m_points;
    Logging::LoggerRef logger;
  };
}
