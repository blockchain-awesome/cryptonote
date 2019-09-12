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
  account_keys_t keys;
  SynchronizationStart syncStart;
  size_t transactionSpendableAge;
};

class ITransfersSubscription;

class ITransfersObserver {
public:
  virtual void onError(ITransfersSubscription* object,
    uint32_t height, std::error_code ec) {
  }

  virtual void onTransactionUpdated(ITransfersSubscription* object, const hash_t& transactionHash) {}

  /**
   * \note The sender must guarantee that onTransactionDeleted() is called only after onTransactionUpdated() is called
   * for the same \a transactionHash.
   */
  virtual void onTransactionDeleted(ITransfersSubscription* object, const hash_t& transactionHash) {}
};

class ITransfersSubscription : public IObservable < ITransfersObserver > {
public:
  virtual ~ITransfersSubscription() {}

  virtual account_public_address_t getAddress() = 0;
  virtual ITransfersContainer& getContainer() = 0;
};

class ITransfersSynchronizerObserver {
public:
  virtual void onBlocksAdded(const public_key_t& viewPublicKey, const std::vector<hash_t>& blockHashes) {}
  virtual void onBlockchainDetach(const public_key_t& viewPublicKey, uint32_t blockIndex) {}
  virtual void onTransactionDeleteBegin(const public_key_t& viewPublicKey, hash_t transactionHash) {}
  virtual void onTransactionDeleteEnd(const public_key_t& viewPublicKey, hash_t transactionHash) {}
  virtual void onTransactionUpdated(const public_key_t& viewPublicKey, const hash_t& transactionHash,
    const std::vector<ITransfersContainer*>& containers) {}
};

class ITransfersSynchronizer : public IStreamSerializable {
public:
  virtual ~ITransfersSynchronizer() {}

  virtual ITransfersSubscription& addSubscription(const AccountSubscription& acc) = 0;
  virtual bool removeSubscription(const account_public_address_t& acc) = 0;
  virtual void getSubscriptions(std::vector<account_public_address_t>& subscriptions) = 0;
  // returns nullptr if address is not found
  virtual ITransfersSubscription* getSubscription(const account_public_address_t& acc) = 0;
  virtual std::vector<hash_t> getViewKeyKnownBlocks(const public_key_t& publicViewKey) = 0;
};

}
