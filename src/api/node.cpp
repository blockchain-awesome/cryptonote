#include <future>
#include <iostream>

#include <boost/thread/thread.hpp>

#include "node.h"

namespace api
{
Node::Node(std::string &host, uint16_t port) : m_host(host), m_port(port)
{
  m_node.reset(new cryptonote::NodeRpcProxy(m_host, m_port));
  m_node->addObserver(static_cast<INodeRpcProxyObserver *>(this));
  m_node->addObserver(static_cast<INodeObserver *>(this));
}

// Interface cryptonote::INodeObserver

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
  std::cout << "pool changed!" << std::endl;
}
void Node::blockchainSynchronized(uint32_t height)
{
  std::cout << "synced height updated to :" << height << std::endl;
}
void Node::peerCountUpdated(size_t count)
{
  std::cout << "peer updated to :" << count << std::endl;
}

// End cryptonote::INodeObserver

// Interface cryptonote::IBlockchainSynchronizerObserver

void Node::synchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount)
{
  std::cout << "sync updated!" << std::endl;
  std::cout << "sync updated processed blockes :" << processedBlockCount << std::endl;
  std::cout << "sync updated total blockes :" << totalBlockCount << std::endl;
}

void Node::synchronizationCompleted(std::error_code result)
{
  std::cout << "sync completed: " << result.value() << std::endl;
  try
  {
    m_blockchain->init();
    std::cout << "blockchain inited !" << std::endl;
  }
  catch (std::exception &e)
  {
    std::cout << "Failed to initialize blockchain :" << e.what() << std::endl;
  }
}

// End cryptonote::IBlockchainSynchronizerObserver

// Interface cryptonote::INodeRpcProxyObserver

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

// End cryptonote::INodeRpcProxyObserver

void Node::onTransactionDeleted(cryptonote::ITransfersSubscription *object, const crypto::Hash &transactionHash)
{
  std::cout << "onTransactionDeleted" << std::endl;
}

void Node::onTransactionUpdated(cryptonote::ITransfersSubscription *object, const crypto::Hash &transactionHash)
{
  std::cout << "onTransactionUpdated" << std::endl;
}

void Node::onError(cryptonote::ITransfersSubscription *object,
                   uint32_t height, std::error_code ec)
{
  std::cout << "onError" << std::endl;
}

cryptonote::ITransfersSubscription &Node::initAccount(cryptonote::AccountKeys &keys)
{
  cryptonote::AccountSubscription sub;
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

bool Node::init(cryptonote::Currency &currency, Logging::ILogger &logger)
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
    m_blockchainSync = std::unique_ptr<cryptonote::BlockchainSynchronizer>(new cryptonote::BlockchainSynchronizer(*m_node, currency.genesisBlockHash()));
    m_blockchainSync->addObserver(this);
    m_transfersSync = std::unique_ptr<cryptonote::TransfersSyncronizer>(new cryptonote::TransfersSyncronizer(
        currency, *m_blockchainSync, *m_node));
    // m_blockchainExplorer = std::unique_ptr<cryptonote::BlockchainExplorer>(new cryptonote::BlockchainExplorer(

    //     *m_node, logger));

    cryptonote::RealTimeProvider timeProvider;
    cryptonote::tx_memory_pool *mempool = new cryptonote::tx_memory_pool(currency, *this, timeProvider, logger);

    m_blockchain = std::unique_ptr<cryptonote::Blockchain>(new cryptonote::Blockchain(
        currency,
        *mempool,
        logger));
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

bool Node::checkTransactionInputs(const cryptonote::Transaction &tx, cryptonote::BlockInfo &maxUsedBlock)
{
    std::cout << "checkTransactionInputs" << std::endl;

  return true;
}
bool Node::checkTransactionInputs(const cryptonote::Transaction &tx, cryptonote::BlockInfo &maxUsedBlock, cryptonote::BlockInfo &lastFailed)
{
    std::cout << "checkTransactionInputs" << std::endl;

  return true;
}
bool Node::haveSpentKeyImages(const cryptonote::Transaction &tx)
{
    std::cout << "haveSpentKeyImages" << std::endl;

  return true;
}
bool Node::checkTransactionSize(size_t blobSize)
{
    std::cout << "blobSize" << blobSize << std::endl;

  return true;
}

} // namespace api