#include <future>
#include <iostream>

#include <boost/thread/thread.hpp>

#include "node.h"

namespace api
{
Node::Node(std::string &host, uint16_t port) : m_host(host), m_port(port)
{
  m_node.reset(new CryptoNote::NodeRpcProxy(m_host, m_port));
  m_node->addObserver(static_cast<INodeRpcProxyObserver *>(this));
  m_node->addObserver(static_cast<INodeObserver *>(this));
}

// Interface CryptoNote::INodeObserver

void Node::localBlockchainUpdated(uint32_t height)
{
  std::cout << "local height updated to :" << height << std::endl;
}
void Node::lastKnownBlockHeightUpdated(uint32_t height)
{
  std::cout << "last height updated to :" << height << std::endl;
}
void Node::poolChanged()
{
}
void Node::blockchainSynchronized(uint32_t height)
{
  std::cout << "synced height updated to :" << height << std::endl;
}
void Node::peerCountUpdated(size_t count)
{
  std::cout << "peer updated to :" << count << std::endl;
}

// End CryptoNote::INodeObserver

// Interface CryptoNote::IBlockchainSynchronizerObserver

void Node::synchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount)
{
  std::cout << "sync updated!" << std::endl;
  std::cout << "sync updated processed blockes :" << processedBlockCount << std::endl;
  std::cout << "sync updated total blockes :" << totalBlockCount << std::endl;
}
void Node::synchronizationCompleted(std::error_code result)
{
  std::cout << "sync completed" << result.value() << std::endl;
}

// End CryptoNote::IBlockchainSynchronizerObserver

// Interface CryptoNote::INodeRpcProxyObserver

void Node::connectionStatusUpdated(bool connected)
{
  m_synced = true;
  if (connected)
  {
    std::cout << "connected to :" << m_host << ":" << m_port << std::endl;
  }
  else
  {
    std::cout << "failed to :" << m_host << ":" << m_port << std::endl;
  }
}

// End CryptoNote::INodeRpcProxyObserver

void Node::onTransactionDeleted(CryptoNote::ITransfersSubscription *object, const Crypto::Hash &transactionHash)
{
  std::cout << "onTransactionDeleted" << std::endl;
}

void Node::onTransactionUpdated(CryptoNote::ITransfersSubscription *object, const Crypto::Hash &transactionHash)
{
  std::cout << "onTransactionUpdated" << std::endl;
}

void Node::onError(CryptoNote::ITransfersSubscription *object,
                   uint32_t height, std::error_code ec)
{
  std::cout << "onError" << std::endl;
}

CryptoNote::ITransfersSubscription &Node::initAccount(CryptoNote::AccountKeys &keys)
{
  CryptoNote::AccountSubscription sub;
  sub.keys = keys;
  sub.transactionSpendableAge = 1;
  sub.syncStart.height = 0;
  sub.syncStart.timestamp = time(0);
  auto &subObject = m_transfersSync->addSubscription(sub);
  // m_transferDetails = &subObject.getContainer();
  subObject.addObserver(this);

  startSync();

  return subObject;

  // m_sender.reset(new WalletTransactionSender(m_currency, m_transactionsCache, m_account.getAccountKeys(), *m_transferDetails));
}

bool Node::init(CryptoNote::Currency &currency)
{
  if (!m_isBlockchainSynced)
  {
    std::promise<std::error_code> errorPromise;
    std::future<std::error_code> error = errorPromise.get_future();
    auto callback = [&errorPromise](std::error_code e) { errorPromise.set_value(e); };
    m_node->init(callback);
    if (error.get())
    {
      return false;
    }
    m_blockchainSync = std::unique_ptr<CryptoNote::BlockchainSynchronizer>(new CryptoNote::BlockchainSynchronizer(*m_node, currency.genesisBlockHash()));
    m_blockchainSync->addObserver(this);
    m_transfersSync = std::unique_ptr<CryptoNote::TransfersSyncronizer>(new CryptoNote::TransfersSyncronizer(
          currency, *m_blockchainSync, *m_node));
  }
  return true;
}

void Node::startSync()
{

  std::thread t([&]() {
    std::cout << "synchronizer started" << std::endl;
    m_blockchainSync->start();
  });

  t.detach();
}

size_t Node::getPeerCount()
{
  if (m_connected)
  {
    std::cout << "inside connected" << std::endl;
    return m_node->getPeerCount();
  }
  return 0;
}

void Node::wait(size_t milliseconds)
{
  std::cout << "start waiting" << std::endl;

  {
    boost::this_thread::sleep(boost::posix_time::milliseconds(milliseconds));
  }
  std::cout << "end waiting" << std::endl;
}
} // namespace api