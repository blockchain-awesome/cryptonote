// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "IBlockchainSynchronizer.h"
#include "ITransfersSynchronizer.h"
#include "TransfersSubscription.h"
#include "TypeHelpers.h"

#include "cryptonote/crypto/crypto.h"

#include "IObservableImpl.h"

#include <unordered_set>

namespace cryptonote {

class INode;

class TransfersConsumer: public IObservableImpl<IBlockchainConsumerObserver, IBlockchainConsumer> {
public:

  TransfersConsumer(const cryptonote::Currency& currency, INode& node, const secret_key_t& viewSecret);

  ITransfersSubscription& addSubscription(const AccountSubscription& subscription);
  // returns true if no subscribers left
  bool removeSubscription(const account_public_address_t& address);
  ITransfersSubscription* getSubscription(const account_public_address_t& acc);
  void getSubscriptions(std::vector<account_public_address_t>& subscriptions);

  void initTransactionPool(const std::unordered_set<hash_t>& uncommitedTransactions);
  
  // IBlockchainConsumer
  virtual SynchronizationStart getSyncStart() override;
  virtual void onBlockchainDetach(uint32_t height) override;
  virtual bool onNewBlocks(const CompleteBlock* blocks, uint32_t startHeight, uint32_t count) override;
  virtual std::error_code onPoolUpdated(const std::vector<std::unique_ptr<ITransactionReader>>& addedTransactions, const std::vector<hash_t>& deletedTransactions) override;
  virtual const std::unordered_set<hash_t>& getKnownPoolTxIds() const override;

  virtual std::error_code addUnconfirmedTransaction(const ITransactionReader& transaction) override;
  virtual void removeUnconfirmedTransaction(const hash_t& transactionHash) override;

private:

  template <typename F>
  void forEachSubscription(F action) {
    for (const auto& kv : m_subscriptions) {
      action(*kv.second);
    }
  }

  struct PreprocessInfo {
    std::unordered_map<public_key_t, std::vector<TransactionOutputInformationIn>> outputs;
    std::vector<uint32_t> globalIdxs;
  };

  std::error_code preprocessOutputs(const TransactionBlockInfo& blockInfo, const ITransactionReader& tx, PreprocessInfo& info);
  std::error_code processTransaction(const TransactionBlockInfo& blockInfo, const ITransactionReader& tx);
  void processTransaction(const TransactionBlockInfo& blockInfo, const ITransactionReader& tx, const PreprocessInfo& info);
  void processOutputs(const TransactionBlockInfo& blockInfo, TransfersSubscription& sub, const ITransactionReader& tx,
    const std::vector<TransactionOutputInformationIn>& outputs, const std::vector<uint32_t>& globalIdxs, bool& contains, bool& updated);

  std::error_code getGlobalIndices(const hash_t& transactionHash, std::vector<uint32_t>& outsGlobalIndices);

  void updateSyncStart();

  SynchronizationStart m_syncStart;
  const secret_key_t m_viewSecret;
  // map { spend public key -> subscription }
  std::unordered_map<public_key_t, std::unique_ptr<TransfersSubscription>> m_subscriptions;
  std::unordered_set<public_key_t> m_spendKeys;
  std::unordered_set<hash_t> m_poolTxs;

  INode& m_node;
  const cryptonote::Currency& m_currency;
};

}
