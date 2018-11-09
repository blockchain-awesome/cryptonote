// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote/core/key.h"
#include "cryptonote/core/Difficulty.h"

namespace cryptonote {
  struct IMinerHandler {
    virtual bool handle_block_found(block_t& b) = 0;
    virtual bool get_block_template(block_t& b, const account_public_address_t& adr, difficulty_t& diffic, uint32_t& height, const binary_array_t& ex_nonce) = 0;

  protected:
    ~IMinerHandler(){};
  };
}
