// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransfersConsumer.h"

#include <numeric>

#include "CommonTypes.h"
#include "common/BlockingQueue.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/TransactionApi.h"

#include "IWallet.h"
#include "INode.h"
#include <future>

using namespace crypto;

namespace {

using namespace cryptonote;

void checkOutputKey(
  const key_derivation_t& derivation,
  const public_key_t& key,
  size_t keyIndex,
  size_t outputIndex,
  const std::unordered_set<public_key_t>& spendKeys,
  std::unordered_map<public_key_t, std::vector<uint32_t>>& outputs) {

  public_key_t spendKey;
  underive_public_key(derivation, keyIndex, key, spendKey);

  if (spendKeys.find(spendKey) != spendKeys.end()) {
    outputs[spendKey].push_back(static_cast<uint32_t>(outputIndex));
  }

}

void findMyOutputs(
  const ITransactionReader& tx,
  const secret_key_t& viewSecretKey,
  const std::unordered_set<public_key_t>& spendKeys,
  std::unordered_map<public_key_t, std::vector<uint32_t>>& outputs) {

  auto txPublicKey = tx.getTransactionPublicKey();
  key_derivation_t derivation;

  if (!generate_key_derivation( txPublicKey, viewSecretKey, derivation)) {
    return;
  }

  size_t keyIndex = 0;
  size_t outputCount = tx.getOutputCount();

  for (size_t idx = 0; idx < outputCount; ++idx) {

    auto outType = tx.getOutputType(size_t(idx));

    if (outType == TransactionTypes::output_type_t::Key) {

      uint64_t amount;
      key_output_t out;
      tx.getOutput(idx, out, amount);
      checkOutputKey(derivation, out.key, keyIndex, idx, spendKeys, outputs);
      ++keyIndex;

    } else if (outType == TransactionTypes::output_type_t::Multisignature) {

      uint64_t amount;
      multi_signature_output_t out;
      tx.getOutput(idx, out, amount);
      for (const auto& key : out.keys) {
        checkOutputKey(derivation, key, idx, idx, spendKeys, outputs);
        ++keyIndex;
      }
    }
  }
}

std::vector<crypto::hash_t> getBlockHashes(const cryptonote::CompleteBlock* blocks, size_t count) {
  std::vector<crypto::hash_t> result;
  result.reserve(count);

  for (size_t i = 0; i < count; ++i) {
    result.push_back(blocks[i].blockHash);
  }

  return result;
}

}

namespace cryptonote {

TransfersConsumer::TransfersConsumer(const cryptonote::Currency& currency, INode& node, const secret_key_t& viewSecret) :
  m_node(node), m_viewSecret(viewSecret), m_currency(currency) {
  updateSyncStart();
}

ITransfersSubscription& TransfersConsumer::addSubscription(const AccountSubscription& subscription) {
  if (subscription.keys.viewSecretKey != m_viewSecret) {
    throw std::runtime_error("TransfersConsumer: view secret key mismatch");
  }

  auto& res = m_subscriptions[subscription.keys.address.spendPublicKey];

  if (res.get() == nullptr) {
    res.reset(new TransfersSubscription(m_currency, subscription));
    m_spendKeys.insert(subscription.keys.address.spendPublicKey);
    updateSyncStart();
  }

  return *res;
}

bool TransfersConsumer::removeSubscription(const account_public_address_t& address) {
  m_subscriptions.erase(address.spendPublicKey);
  m_spendKeys.erase(address.spendPublicKey);
  updateSyncStart();
  return m_subscriptions.empty();
}

ITransfersSubscription* TransfersConsumer::getSubscription(const account_public_address_t& acc) {
  auto it = m_subscriptions.find(acc.spendPublicKey);
  return it == m_subscriptions.end() ? nullptr : it->second.get();
}

void TransfersConsumer::getSubscriptions(std::vector<account_public_address_t>& subscriptions) {
  for (const auto& kv : m_subscriptions) {
    subscriptions.push_back(kv.second->getAddress());
  }
}

void TransfersConsumer::initTransactionPool(const std::unordered_set<crypto::hash_t>& uncommitedTransactions) {
  for (auto itSubscriptions = m_subscriptions.begin(); itSubscriptions != m_subscriptions.end(); ++itSubscriptions) {
    std::vector<crypto::hash_t> unconfirmedTransactions;
    itSubscriptions->second->getContainer().getUnconfirmedTransactions(unconfirmedTransactions);

    for (auto itTransactions = unconfirmedTransactions.begin(); itTransactions != unconfirmedTransactions.end(); ++itTransactions) {
      if (uncommitedTransactions.count(*itTransactions) == 0) {
        m_poolTxs.emplace(*itTransactions);
      }
    }
  }
}

void TransfersConsumer::updateSyncStart() {
  SynchronizationStart start;

  start.height =   std::numeric_limits<uint64_t>::max();
  start.timestamp = std::numeric_limits<uint64_t>::max();

  for (const auto& kv : m_subscriptions) {
    auto subStart = kv.second->getSyncStart();
    start.height = std::min(start.height, subStart.height);
    start.timestamp = std::min(start.timestamp, subStart.timestamp);
  }

  m_syncStart = start;
}

SynchronizationStart TransfersConsumer::getSyncStart() {
  return m_syncStart;
}

void TransfersConsumer::onBlockchainDetach(uint32_t height) {
  m_observerManager.notify(&IBlockchainConsumerObserver::onBlockchainDetach, this, height);

  for (const auto& kv : m_subscriptions) {
    kv.second->onBlockchainDetach(height);
  }
}

bool TransfersConsumer::onNewBlocks(const CompleteBlock* blocks, uint32_t startHeight, uint32_t count) {
  assert(blocks);
  assert(count > 0);

  struct Tx {
    TransactionBlockInfo blockInfo;
    const ITransactionReader* tx;
  };

  struct PreprocessedTx : Tx, PreprocessInfo {};

  std::vector<PreprocessedTx> preprocessedTransactions;
  std::mutex preprocessedTransactionsMutex;

  size_t workers = std::thread::hardware_concurrency();
  if (workers == 0) {
    workers = 2;
  }

  BlockingQueue<Tx> inputQueue(workers * 2);

  std::atomic<bool> stopProcessing(false);

  auto pushingThread = std::async(std::launch::async, [&] {
    for( uint32_t i = 0; i < count && !stopProcessing; ++i) {
      const auto& block = blocks[i].block;

      if (!block.is_initialized()) {
        continue;
      }

      // filter by syncStartTimestamp
      if (m_syncStart.timestamp && block->timestamp < m_syncStart.timestamp) {
        continue;
      }

      TransactionBlockInfo blockInfo;
      blockInfo.height = startHeight + i;
      blockInfo.timestamp = block->timestamp;
      blockInfo.transactionIndex = 0; // position in block

      for (const auto& tx : blocks[i].transactions) {
        auto pubKey = tx->getTransactionPublicKey();
        if (pubKey == NULL_PUBLIC_KEY) {
          ++blockInfo.transactionIndex;
          continue;
        }

        Tx item = { blockInfo, tx.get() };
        inputQueue.push(item);
        ++blockInfo.transactionIndex;
      }
    }

    inputQueue.close();
  });

  auto processingFunction = [&] {
    Tx item;
    std::error_code ec;
    while (!stopProcessing && inputQueue.pop(item)) {
      PreprocessedTx output;
      static_cast<Tx&>(output) = item;

      ec = preprocessOutputs(item.blockInfo, *item.tx, output);
      if (ec) {
        stopProcessing = true;
        break;
      }

      std::lock_guard<std::mutex> lk(preprocessedTransactionsMutex);
      preprocessedTransactions.push_back(std::move(output));
    }
    return ec;
  };

  std::vector<std::future<std::error_code>> processingThreads;
  for (size_t i = 0; i < workers; ++i) {
    processingThreads.push_back(std::async(std::launch::async, processingFunction));
  }

  std::error_code processingError;
  for (auto& f : processingThreads) {
    try {
      std::error_code ec = f.get();
      if (!processingError && ec) {
        processingError = ec;
      }
    } catch (const std::system_error& e) {
      processingError = e.code();
    } catch (const std::exception&) {
      processingError = std::make_error_code(std::errc::operation_canceled);
    }
  }

  std::vector<crypto::hash_t> blockHashes = getBlockHashes(blocks, count);
  if (!processingError) {
    m_observerManager.notify(&IBlockchainConsumerObserver::onBlocksAdded, this, blockHashes);

    // sort by block height and transaction index in block
    std::sort(preprocessedTransactions.begin(), preprocessedTransactions.end(), [](const PreprocessedTx& a, const PreprocessedTx& b) {
      return std::tie(a.blockInfo.height, a.blockInfo.transactionIndex) < std::tie(b.blockInfo.height, b.blockInfo.transactionIndex);
    });

    for (const auto& tx : preprocessedTransactions) {
      processTransaction(tx.blockInfo, *tx.tx, tx);
    }
  } else {
    forEachSubscription([&](TransfersSubscription& sub) {
      sub.onError(processingError, startHeight);
    });

    return false;
  }

  auto newHeight = startHeight + count - 1;
  forEachSubscription([newHeight](TransfersSubscription& sub) {
    sub.advanceHeight(newHeight);
  });

  return true;
}

std::error_code TransfersConsumer::onPoolUpdated(const std::vector<std::unique_ptr<ITransactionReader>>& addedTransactions, const std::vector<hash_t>& deletedTransactions) {
  TransactionBlockInfo unconfirmedBlockInfo;
  unconfirmedBlockInfo.timestamp = 0; 
  unconfirmedBlockInfo.height = WALLET_UNCONFIRMED_TRANSACTION_HEIGHT;

  std::error_code processingError;
  for (auto& cryptonoteTransaction : addedTransactions) {
    m_poolTxs.emplace(cryptonoteTransaction->getTransactionHash());
    processingError = processTransaction(unconfirmedBlockInfo, *cryptonoteTransaction.get());
    if (processingError) {
      for (auto& sub : m_subscriptions) {
        sub.second->onError(processingError, WALLET_UNCONFIRMED_TRANSACTION_HEIGHT);
      }

      return processingError;
    }
  }
  
  for (auto& deletedTxHash : deletedTransactions) {
    m_poolTxs.erase(deletedTxHash);

    m_observerManager.notify(&IBlockchainConsumerObserver::onTransactionDeleteBegin, this, deletedTxHash);
    for (auto& sub : m_subscriptions) {
      sub.second->deleteUnconfirmedTransaction(*reinterpret_cast<const hash_t*>(&deletedTxHash));
    }

    m_observerManager.notify(&IBlockchainConsumerObserver::onTransactionDeleteEnd, this, deletedTxHash);
  }

  return std::error_code();
}

const std::unordered_set<crypto::hash_t>& TransfersConsumer::getKnownPoolTxIds() const {
  return m_poolTxs;
}

std::error_code TransfersConsumer::addUnconfirmedTransaction(const ITransactionReader& transaction) {
  TransactionBlockInfo unconfirmedBlockInfo;
  unconfirmedBlockInfo.height = WALLET_UNCONFIRMED_TRANSACTION_HEIGHT;
  unconfirmedBlockInfo.timestamp = 0;
  unconfirmedBlockInfo.transactionIndex = 0;

  return processTransaction(unconfirmedBlockInfo, transaction);
}

void TransfersConsumer::removeUnconfirmedTransaction(const crypto::hash_t& transactionHash) {
  m_observerManager.notify(&IBlockchainConsumerObserver::onTransactionDeleteBegin, this, transactionHash);
  for (auto& subscription : m_subscriptions) {
    subscription.second->deleteUnconfirmedTransaction(transactionHash);
  }
  m_observerManager.notify(&IBlockchainConsumerObserver::onTransactionDeleteEnd, this, transactionHash);
}

std::error_code createTransfers(
  const account_keys_t& account,
  const TransactionBlockInfo& blockInfo,
  const ITransactionReader& tx,
  const std::vector<uint32_t>& outputs,
  const std::vector<uint32_t>& globalIdxs,
  std::vector<TransactionOutputInformationIn>& transfers) {

  auto txPubKey = tx.getTransactionPublicKey();

  for (auto idx : outputs) {

    if (idx >= tx.getOutputCount()) {
      return std::make_error_code(std::errc::argument_out_of_domain);
    }

    auto outType = tx.getOutputType(size_t(idx));

    if (
      outType != TransactionTypes::output_type_t::Key &&
      outType != TransactionTypes::output_type_t::Multisignature) {
      continue;
    }

    TransactionOutputInformationIn info;

    info.type = outType;
    info.transactionPublicKey = txPubKey;
    info.outputInTransaction = idx;
    info.globalOutputIndex = (blockInfo.height == WALLET_UNCONFIRMED_TRANSACTION_HEIGHT) ?
      UNCONFIRMED_TRANSACTION_GLOBAL_OUTPUT_INDEX : globalIdxs[idx];

    if (outType == TransactionTypes::output_type_t::Key) {
      uint64_t amount;
      key_output_t out;
      tx.getOutput(idx, out, amount);

      cryptonote::key_pair_t in_ephemeral;
      cryptonote::generate_key_image_helper(
        account,
        txPubKey,
        idx,
        in_ephemeral,
        info.keyImage);

      assert(out.key == reinterpret_cast<const public_key_t&>(in_ephemeral.publicKey));

      info.amount = amount;
      info.outputKey = out.key;

    } else if (outType == TransactionTypes::output_type_t::Multisignature) {
      uint64_t amount;
      multi_signature_output_t out;
      tx.getOutput(idx, out, amount);

      info.amount = amount;
      info.requiredSignatures = out.requiredSignatureCount;
    }

    transfers.push_back(info);
  }

  return std::error_code();
}

std::error_code TransfersConsumer::preprocessOutputs(const TransactionBlockInfo& blockInfo, const ITransactionReader& tx, PreprocessInfo& info) {
  std::unordered_map<public_key_t, std::vector<uint32_t>> outputs;
  findMyOutputs(tx, m_viewSecret, m_spendKeys, outputs);

  if (outputs.empty()) {
    return std::error_code();
  }

  std::error_code errorCode;
  auto txHash = tx.getTransactionHash();
  if (blockInfo.height != WALLET_UNCONFIRMED_TRANSACTION_HEIGHT) {
    errorCode = getGlobalIndices(reinterpret_cast<const hash_t&>(txHash), info.globalIdxs);
    if (errorCode) {
      return errorCode;
    }
  }

  for (const auto& kv : outputs) {
    auto it = m_subscriptions.find(kv.first);
    if (it != m_subscriptions.end()) {
      auto& transfers = info.outputs[kv.first];
      errorCode = createTransfers(it->second->getKeys(), blockInfo, tx, kv.second, info.globalIdxs, transfers);
      if (errorCode) {
        return errorCode;
      }
    }
  }

  return std::error_code();
}

std::error_code TransfersConsumer::processTransaction(const TransactionBlockInfo& blockInfo, const ITransactionReader& tx) {
  PreprocessInfo info;
  auto ec = preprocessOutputs(blockInfo, tx, info);
  if (ec) {
    return ec;
  }

  processTransaction(blockInfo, tx, info);
  return std::error_code();
}

void TransfersConsumer::processTransaction(const TransactionBlockInfo& blockInfo, const ITransactionReader& tx, const PreprocessInfo& info) {
  std::vector<TransactionOutputInformationIn> emptyOutputs;
  std::vector<ITransfersContainer*> transactionContainers;
  bool someContainerUpdated = false;
  for (auto& kv : m_subscriptions) {
    auto it = info.outputs.find(kv.first);
    auto& subscriptionOutputs = (it == info.outputs.end()) ? emptyOutputs : it->second;

    bool containerContainsTx;
    bool containerUpdated;
    processOutputs(blockInfo, *kv.second, tx, subscriptionOutputs, info.globalIdxs, containerContainsTx, containerUpdated);
    someContainerUpdated = someContainerUpdated || containerUpdated;
    if (containerContainsTx) {
      transactionContainers.emplace_back(&kv.second->getContainer());
    }
  }

  if (someContainerUpdated) {
    m_observerManager.notify(&IBlockchainConsumerObserver::onTransactionUpdated, this, tx.getTransactionHash(), transactionContainers);
  }
}

void TransfersConsumer::processOutputs(const TransactionBlockInfo& blockInfo, TransfersSubscription& sub, const ITransactionReader& tx,
  const std::vector<TransactionOutputInformationIn>& transfers, const std::vector<uint32_t>& globalIdxs, bool& contains, bool& updated) {

  TransactionInformation subscribtionTxInfo;
  contains = sub.getContainer().getTransactionInformation(tx.getTransactionHash(), subscribtionTxInfo);
  updated = false;

  if (contains) {
    if (subscribtionTxInfo.blockHeight == WALLET_UNCONFIRMED_TRANSACTION_HEIGHT && blockInfo.height != WALLET_UNCONFIRMED_TRANSACTION_HEIGHT) {
      // pool->blockchain
      sub.markTransactionConfirmed(blockInfo, tx.getTransactionHash(), globalIdxs);
      updated = true;
    } else {
      assert(subscribtionTxInfo.blockHeight == blockInfo.height);
    }
  } else {
    updated = sub.addTransaction(blockInfo, tx, transfers);
    contains = updated;
  }
}

std::error_code TransfersConsumer::getGlobalIndices(const hash_t& transactionHash, std::vector<uint32_t>& outsGlobalIndices) {  
  std::promise<std::error_code> prom;
  std::future<std::error_code> f = prom.get_future();

  INode::Callback cb = [&prom](std::error_code ec) { 
    std::promise<std::error_code> p(std::move(prom));
    p.set_value(ec);
  };

  outsGlobalIndices.clear();
  m_node.getTransactionOutsGlobalIndices(transactionHash, outsGlobalIndices, cb);

  return f.get();
}

}
