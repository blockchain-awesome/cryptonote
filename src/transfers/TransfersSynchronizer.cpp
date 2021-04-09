// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransfersSynchronizer.h"
#include "TransfersConsumer.h"

#include "stream/crypto.h"
#include "stream/cryptonote.h"
#include "stream/block.h"

using namespace Common;
using namespace crypto;

namespace cryptonote {

const uint32_t TRANSFERS_STORAGE_ARCHIVE_VERSION = 0;

TransfersSyncronizer::TransfersSyncronizer(const cryptonote::Currency& currency, IBlockchainSynchronizer& sync, INode& node) :
  m_currency(currency), m_sync(sync), m_node(node) {
}

TransfersSyncronizer::~TransfersSyncronizer() {
  m_sync.stop();
  for (const auto& kv : m_consumers) {
    m_sync.removeConsumer(kv.second.get());
  }
}

void TransfersSyncronizer::initTransactionPool(const std::unordered_set<hash_t>& uncommitedTransactions) {
  for (auto it = m_consumers.begin(); it != m_consumers.end(); ++it) {
    it->second->initTransactionPool(uncommitedTransactions);
  }
}

ITransfersSubscription& TransfersSyncronizer::addSubscription(const AccountSubscription& acc) {
  auto it = m_consumers.find(acc.keys.address.viewPublicKey);

  if (it == m_consumers.end()) {
    std::unique_ptr<TransfersConsumer> consumer(
      new TransfersConsumer(m_currency, m_node, acc.keys.viewSecretKey));

    m_sync.addConsumer(consumer.get());
    consumer->addObserver(this);
    it = m_consumers.insert(std::make_pair(acc.keys.address.viewPublicKey, std::move(consumer))).first;
  }
    
  return it->second->addSubscription(acc);
}

bool TransfersSyncronizer::removeSubscription(const account_public_address_t& acc) {
  auto it = m_consumers.find(acc.viewPublicKey);
  if (it == m_consumers.end())
    return false;

  if (it->second->removeSubscription(acc)) {
    m_sync.removeConsumer(it->second.get());
    m_consumers.erase(it);

    m_subscribers.erase(acc.viewPublicKey);
  }

  return true;
}

void TransfersSyncronizer::getSubscriptions(std::vector<account_public_address_t>& subscriptions) {
  for (const auto& kv : m_consumers) {
    kv.second->getSubscriptions(subscriptions);
  }
}

ITransfersSubscription* TransfersSyncronizer::getSubscription(const account_public_address_t& acc) {
  auto it = m_consumers.find(acc.viewPublicKey);
  return (it == m_consumers.end()) ? nullptr : it->second->getSubscription(acc);
}

std::vector<hash_t> TransfersSyncronizer::getViewKeyKnownBlocks(const public_key_t& publicViewKey) {
  auto it = m_consumers.find(publicViewKey);
  if (it == m_consumers.end()) {
    throw std::invalid_argument("Consumer not found");
  }

  return m_sync.getConsumerKnownBlocks(*it->second);
}

void TransfersSyncronizer::onBlocksAdded(IBlockchainConsumer* consumer, const std::vector<hash_t>& blockHashes) {
  auto it = findSubscriberForConsumer(consumer);
  if (it != m_subscribers.end()) {
    it->second->notify(&ITransfersSynchronizerObserver::onBlocksAdded, it->first, blockHashes);
  }
}

void TransfersSyncronizer::onBlockchainDetach(IBlockchainConsumer* consumer, uint32_t blockIndex) {
  auto it = findSubscriberForConsumer(consumer);
  if (it != m_subscribers.end()) {
    it->second->notify(&ITransfersSynchronizerObserver::onBlockchainDetach, it->first, blockIndex);
  }
}

void TransfersSyncronizer::onTransactionDeleteBegin(IBlockchainConsumer* consumer, hash_t transactionHash) {
  auto it = findSubscriberForConsumer(consumer);
  if (it != m_subscribers.end()) {
    it->second->notify(&ITransfersSynchronizerObserver::onTransactionDeleteBegin, it->first, transactionHash);
  }
}

void TransfersSyncronizer::onTransactionDeleteEnd(IBlockchainConsumer* consumer, hash_t transactionHash) {
  auto it = findSubscriberForConsumer(consumer);
  if (it != m_subscribers.end()) {
    it->second->notify(&ITransfersSynchronizerObserver::onTransactionDeleteEnd, it->first, transactionHash);
  }
}

void TransfersSyncronizer::onTransactionUpdated(IBlockchainConsumer* consumer, const hash_t& transactionHash,
  const std::vector<ITransfersContainer*>& containers) {

  auto it = findSubscriberForConsumer(consumer);
  if (it != m_subscribers.end()) {
    it->second->notify(&ITransfersSynchronizerObserver::onTransactionUpdated, it->first, transactionHash, containers);
  }
}

void TransfersSyncronizer::subscribeConsumerNotifications(const public_key_t& viewPublicKey, ITransfersSynchronizerObserver* observer) {
  auto it = m_subscribers.find(viewPublicKey);
  if (it != m_subscribers.end()) {
    it->second->add(observer);
    return;
  }

  auto insertedIt = m_subscribers.emplace(viewPublicKey, std::unique_ptr<SubscribersNotifier>(new SubscribersNotifier())).first;
  insertedIt->second->add(observer);
}

void TransfersSyncronizer::unsubscribeConsumerNotifications(const public_key_t& viewPublicKey, ITransfersSynchronizerObserver* observer) {
  m_subscribers.at(viewPublicKey)->remove(observer);
}

void TransfersSyncronizer::save(std::ostream& os) {
  m_sync.save(os);

  Writer stream(os);
  stream << const_cast<uint32_t&>(TRANSFERS_STORAGE_ARCHIVE_VERSION);

  size_t subscriptionCount = m_consumers.size();

  stream << subscriptionCount;

  for (const auto& consumer : m_consumers) {

    stream << const_cast<const public_key_t&>(consumer.first);

    std::stringstream consumerState;
    // synchronization state
    m_sync.getConsumerState(consumer.second.get())->save(consumerState);

    std::string blob = consumerState.str();
    stream << blob;    
    std::vector<account_public_address_t> subscriptions;
    consumer.second->getSubscriptions(subscriptions);
    size_t subCount = subscriptions.size();

    stream << subCount;

    for (auto& addr : subscriptions) {
      auto sub = consumer.second->getSubscription(addr);
      if (sub != nullptr) {
        std::stringstream subState;
        assert(sub);
        sub->getContainer().save(subState);
        // store data block
        std::string blob = subState.str();
        stream << addr;
        stream << blob;

      }
    }
  }
}

namespace {
std::string getObjectState(IStreamSerializable& obj) {
  std::stringstream stream;
  obj.save(stream);
  return stream.str();
}

void setObjectState(IStreamSerializable& obj, const std::string& state) {
  std::stringstream stream(state);
  obj.load(stream);
}

}

void TransfersSyncronizer::load(std::istream& is) {
  m_sync.load(is);

  Reader i(is);
  uint32_t version = 0;

  i >> version;

  if (version > TRANSFERS_STORAGE_ARCHIVE_VERSION) {
    throw std::runtime_error("TransfersSyncronizer version mismatch");
  }


  struct ConsumerState {
    public_key_t viewKey;
    std::string state;
    std::vector<std::pair<account_public_address_t, std::string>> subscriptionStates;
  };

  std::vector<ConsumerState> updatedStates;

  try {
    size_t subscriptionCount = 0;

    i >> subscriptionCount;

    while (subscriptionCount--) {
      public_key_t viewKey;
      i >> viewKey;

      std::string blob;

      i >> blob;

      auto subIter = m_consumers.find(viewKey);
      if (subIter != m_consumers.end()) {
        auto consumerState = m_sync.getConsumerState(subIter->second.get());
        assert(consumerState);

        {
          // store previous state
          auto prevConsumerState = getObjectState(*consumerState);
          // load consumer state
          setObjectState(*consumerState, blob);
          updatedStates.push_back(ConsumerState{ viewKey, std::move(prevConsumerState) });
        }

        // load subscriptions
        size_t subCount = 0;
        i >> subCount;

        while (subCount--) {

          account_public_address_t acc;
          std::string state;

          i >> acc;
          i >> state;

          auto sub = subIter->second->getSubscription(acc);

          if (sub != nullptr) {
            auto prevState = getObjectState(sub->getContainer());
            setObjectState(sub->getContainer(), state);
            updatedStates.back().subscriptionStates.push_back(std::make_pair(acc, prevState));
          }

        }
      }
    }
  } catch (...) {
    // rollback state
    for (const auto& consumerState : updatedStates) {
      auto consumer = m_consumers.find(consumerState.viewKey)->second.get();
      setObjectState(*m_sync.getConsumerState(consumer), consumerState.state);
      for (const auto& sub : consumerState.subscriptionStates) {
        setObjectState(consumer->getSubscription(sub.first)->getContainer(), sub.second);
      }
    }
    throw;
  }

}

bool TransfersSyncronizer::findViewKeyForConsumer(IBlockchainConsumer* consumer, public_key_t& viewKey) const {
  //since we have only couple of consumers linear complexity is fine
  auto it = std::find_if(m_consumers.begin(), m_consumers.end(), [consumer] (const ConsumersContainer::value_type& subscription) {
    return subscription.second.get() == consumer;
  });

  if (it == m_consumers.end()) {
    return false;
  }

  viewKey = it->first;
  return true;
}

TransfersSyncronizer::SubscribersContainer::const_iterator TransfersSyncronizer::findSubscriberForConsumer(IBlockchainConsumer* consumer) const {
  public_key_t viewKey;
  if (findViewKeyForConsumer(consumer, viewKey)) {
    auto it = m_subscribers.find(viewKey);
    if (it != m_subscribers.end()) {
      return it;
    }
  }

  return m_subscribers.end();
}

}
