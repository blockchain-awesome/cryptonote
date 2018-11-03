// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>
#include <system_error>

#include "ITransaction.h"
#include "ITransfersContainer.h"
#include "IStreamSerializable.h"

namespace cryptonote {

struct SynchronizationStart {
  uint64_t timestamp;
  uint64_t height;
};

struct AccountSubscription {
  AccountKeys keys;
  SynchronizationStart syncStart;
  size_t transactionSpendableAge;
};

class ITransfersSubscription;

class ITransfersObserver {
public:
  virtual void onError(ITransfersSubscription* object,
    uint32_t height, std::error_code ec) {
  }

  virtual void onTransactionUpdated(ITransfersSubscription* object, const crypto::Hash& transactionHash) {}

  /**
   * \note The sender must guarantee that onTransactionDeleted() is called only after onTransactionUpdated() is called
   * for the same \a transactionHash.
   */
  virtual void onTransactionDeleted(ITransfersSubscription* object, const crypto::Hash& transactionHash) {}
};

class ITransfersSubscription : public IObservable < ITransfersObserver > {
public:
  virtual ~ITransfersSubscription() {}

  virtual AccountPublicAddress getAddress() = 0;
  virtual ITransfersContainer& getContainer() = 0;
};

class ITransfersSynchronizerObserver {
public:
  virtual void onBlocksAdded(const crypto::public_key_t& viewPublicKey, const std::vector<crypto::Hash>& blockHashes) {}
  virtual void onBlockchainDetach(const crypto::public_key_t& viewPublicKey, uint32_t blockIndex) {}
  virtual void onTransactionDeleteBegin(const crypto::public_key_t& viewPublicKey, crypto::Hash transactionHash) {}
  virtual void onTransactionDeleteEnd(const crypto::public_key_t& viewPublicKey, crypto::Hash transactionHash) {}
  virtual void onTransactionUpdated(const crypto::public_key_t& viewPublicKey, const crypto::Hash& transactionHash,
    const std::vector<ITransfersContainer*>& containers) {}
};

class ITransfersSynchronizer : public IStreamSerializable {
public:
  virtual ~ITransfersSynchronizer() {}

  virtual ITransfersSubscription& addSubscription(const AccountSubscription& acc) = 0;
  virtual bool removeSubscription(const AccountPublicAddress& acc) = 0;
  virtual void getSubscriptions(std::vector<AccountPublicAddress>& subscriptions) = 0;
  // returns nullptr if address is not found
  virtual ITransfersSubscription* getSubscription(const AccountPublicAddress& acc) = 0;
  virtual std::vector<crypto::Hash> getViewKeyKnownBlocks(const crypto::public_key_t& publicViewKey) = 0;
};

}
