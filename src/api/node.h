#pragma once

#include <string>
#include <cstdint>

#include "NodeRpcProxy/NodeRpcProxy.h"
#include "Transfers/BlockchainSynchronizer.h"
#include "Transfers/TransfersSynchronizer.h"
#include "ITransfersSynchronizer.h"

#include <CryptoNoteCore/Account.h>
#include <CryptoNoteCore/Currency.h>

namespace api
{
class Node : public CryptoNote::INodeObserver,
             public CryptoNote::INodeRpcProxyObserver,
             public CryptoNote::IBlockchainSynchronizerObserver,
            //  public CryptoNote::IBlockchainSynchronizer,
             public CryptoNote::ITransfersObserver
{

public:
  Node(std::string &host, uint16_t port);
  CryptoNote::ITransfersSubscription &initAccount(CryptoNote::TransfersSyncronizer &transferSync, CryptoNote::AccountKeys &keys);
  bool init(CryptoNote::Currency &currency);
  void wait(size_t milliseconds = 1000);

  void startSync();

  CryptoNote::NodeRpcProxy &getNode()
  {
    return *m_node;
  }

  size_t getPeerCount();

  // Interface CryptoNote::INodeObserver
  virtual void peerCountUpdated(size_t count) override;
  virtual void localBlockchainUpdated(uint32_t height) override;
  virtual void lastKnownBlockHeightUpdated(uint32_t height) override;
  virtual void poolChanged() override;
  virtual void blockchainSynchronized(uint32_t topHeight) override;

  // Interface CryptoNote::INodeRpcProxyObserver
  virtual void connectionStatusUpdated(bool connected) override;

  // Interface CryptoNote::IBlockchainSynchronizerObserver
  virtual void synchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount) override;
  virtual void synchronizationCompleted(std::error_code result) override;

  // Interface CryptoNote::ITransfersObserver
  virtual void onTransactionDeleted(CryptoNote::ITransfersSubscription *object, const Crypto::Hash &transactionHash) override;

  virtual void onTransactionUpdated(CryptoNote::ITransfersSubscription *object, const Crypto::Hash &transactionHash) override;

  virtual void onError(CryptoNote::ITransfersSubscription *object,
                       uint32_t height, std::error_code ec);

  // Interface CryptoNote::IBlockchainSynchronizer
  // virtual void addObserver(CryptoNote::IBlockchainSynchronizerObserver *observer) override;
  // virtual void removeObserver(CryptoNote::IBlockchainSynchronizerObserver *observer) override;


  // virtual void save(std::ostream& os) override;
  // virtual void load(std::istream& in) override;


  // virtual void addConsumer(CryptoNote::IBlockchainConsumer *consumer) override;
  // virtual bool removeConsumer(CryptoNote::IBlockchainConsumer *consumer) override;
  // virtual IStreamSerializable *getConsumerState(CryptoNote::IBlockchainConsumer *consumer) const override;
  // virtual std::vector<Crypto::Hash> getConsumerKnownBlocks(CryptoNote::IBlockchainConsumer &consumer) const override;

  // virtual std::future<std::error_code> addUnconfirmedTransaction(const CryptoNote::ITransactionReader &transaction) override;
  // virtual std::future<void> removeUnconfirmedTransaction(const Crypto::Hash &transactionHash) override;

  // virtual void start() override;
  // virtual void stop() override;

private:
std::unique_ptr<CryptoNote::NodeRpcProxy> m_node;
std::string m_host;
uint16_t m_port;

bool m_isBlockchainSynced = false;
std::unique_ptr<CryptoNote::BlockchainSynchronizer> m_blockchainSync;
// std::unique_ptr<CryptoNote::TransfersSyncronizer> m_transfersSync;
std::unique_ptr<CryptoNote::Currency> m_currency;

bool m_connected = false;
bool m_synced = false;

std::mutex m_syncMutex;
std::condition_variable m_syncCV;
}; // namespace api

} // namespace api