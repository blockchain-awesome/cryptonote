// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote.h"

namespace cryptonote {
class IBlock {
public:
  virtual ~IBlock();

  virtual const block_t& getBlock() const = 0;
  virtual size_t getTransactionCount() const = 0;
  virtual const transaction_t& getTransaction(size_t index) const = 0;
};
}
