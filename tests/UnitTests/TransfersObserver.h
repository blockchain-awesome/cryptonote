// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <tuple>
#include <vector>
#include "ITransfersSynchronizer.h"

namespace cryptonote {

class TransfersObserver : public ITransfersObserver {
public:
  virtual void onError(ITransfersSubscription* object, uint32_t height, std::error_code ec) override {
    errors.emplace_back(height, ec);
  }

  virtual void onTransactionUpdated(ITransfersSubscription* object, const hash_t& transactionHash) override {
    updated.push_back(transactionHash);
  }

  virtual void onTransactionDeleted(ITransfersSubscription* object, const hash_t& transactionHash) override {
    deleted.push_back(transactionHash);
  }

  std::vector<std::tuple<uint64_t, std::error_code>> errors;
  std::vector<hash_t> updated;
  std::vector<hash_t> deleted;
};


}
