// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <array>
#include <memory>
#include <cstdint>

#include <boost/optional.hpp>

#include "INode.h"
#include "ITransaction.h"

namespace cryptonote {

struct BlockchainInterval {
  uint32_t startHeight;
  std::vector<crypto::hash_t> blocks;
};

struct CompleteBlock {
  crypto::hash_t blockHash;
  boost::optional<cryptonote::block_t> block;
  // first transaction is always coinbase
  std::list<std::shared_ptr<ITransactionReader>> transactions;
};

}
