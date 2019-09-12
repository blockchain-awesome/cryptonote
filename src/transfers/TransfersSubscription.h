// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "ITransfersSynchronizer.h"
#include "TransfersContainer.h"
#include "IObservableImpl.h"

namespace cryptonote {

class TransfersSubscription : public IObservableImpl < ITransfersObserver, ITransfersSubscription > {
public:
  TransfersSubscription(const cryptonote::Currency& currency, const AccountSubscription& sub);

  SynchronizationStart getSyncStart();
  void onBlockchainDetach(uint32_t height);
  void onError(const std::error_code& ec, uint32_t height);
  bool advanceHeight(uint32_t height);
  const account_keys_t& getKeys() const;
  bool addTransaction(const TransactionBlockInfo& blockInfo, const ITransactionReader& tx,
                      const std::vector<TransactionOutputInformationIn>& transfers);

  void deleteUnconfirmedTransaction(const hash_t& transactionHash);
  void markTransactionConfirmed(const TransactionBlockInfo& block, const hash_t& transactionHash, const std::vector<uint32_t>& globalIndices);

  // ITransfersSubscription
  virtual account_public_address_t getAddress() override;
  virtual ITransfersContainer& getContainer() override;

private:
  TransfersContainer transfers;
  AccountSubscription subscription;
};

}
