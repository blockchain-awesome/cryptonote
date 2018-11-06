// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote/core/key.h"
#include "cryptonote/core/blockchain/structures.h"

namespace cryptonote {
  class ITransactionValidator {
  public:
    virtual ~ITransactionValidator() {}
    
    virtual bool checkTransactionInputs(const cryptonote::transaction_t& tx, block_info_t& maxUsedBlock) = 0;
    virtual bool checkTransactionInputs(const cryptonote::transaction_t& tx, block_info_t& maxUsedBlock, block_info_t& lastFailed) = 0;
    virtual bool haveSpentKeyImages(const cryptonote::transaction_t& tx) = 0;
    virtual bool checkTransactionSize(size_t blobSize) = 0;
  };

}
