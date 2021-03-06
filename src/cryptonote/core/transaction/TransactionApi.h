// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <memory>
#include "ITransaction.h"

namespace cryptonote {
  std::unique_ptr<ITransaction> createTransaction();
  std::unique_ptr<ITransaction> createTransaction(const binary_array_t& transactionBlob);
  std::unique_ptr<ITransaction> createTransaction(const transaction_t& tx);

  std::unique_ptr<ITransactionReader> createTransactionPrefix(const transaction_prefix_t& prefix, const hash_t& transactionHash);
  std::unique_ptr<ITransactionReader> createTransactionPrefix(const transaction_t& fullTransaction);
}
