#pragma once

#include <string>
#include <cstdint>

#include "NodeRpcProxy/NodeRpcProxy.h"
#include "transfers/BlockchainSynchronizer.h"
#include "transfers/TransfersSynchronizer.h"
#include "blockchain_explorer/BlockchainExplorer.h"
#include "ITransfersSynchronizer.h"

#include <cryptonote/core/Account.h>
#include <cryptonote/core/Blockchain.h>
#include <cryptonote/core/Currency.h>
#include <logging/ILogger.h>

namespace api
{
class Node : public cryptonote::INodeObserver,
             public cryptonote::INodeRpcProxyObserver,
             public cryptonote::IBlockchainSynchronizerObserver,
             public cryptonote::ITransfersObserver,
             public cryptonote::ITransactionValidator
{

public:
  Node(std::string &host, uint16_t port);
  cryptonote::ITransfersSubscription &initAccount(cryptonote::AccountKeys &keys);
  bool init(cryptonote::Currency &currency, Logging::ILogger &logger);
  void wait(size_t milliseconds = 1000);

  void startSync();

  // Get main objects
  cryptonote::NodeRpcProxy &getNode()
  {
    return *m_node;
  }

  size_t getPeerCount();

  // Interface cryptonote::INodeObserver
  virtual void peerCountUpdated(size_t count) override;
  virtual void localBlockchainUpdated(uint32_t height) override;
  virtual void lastKnownBlockHeightUpdated(uint32_t height) override;
  virtual void poolChanged() override;
  virtual void blockchainSynchronized(uint32_t topHeight) override;

  // Interface cryptonote::INodeRpcProxyObserver
  virtual void connectionStatusUpdated(bool connected) override;

  // Interface cryptonote::IBlockchainSynchronizerObserver
  virtual void synchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount) override;
  virtual void synchronizationCompleted(std::error_code result) override;

  // Interface cryptonote::ITransfersObserver
  virtual void onTransactionDeleted(cryptonote::ITransfersSubscription *object, const crypto::Hash &transactionHash) override;

  virtual void onTransactionUpdated(cryptonote::ITransfersSubscription *object, const crypto::Hash &transactionHash) override;

  virtual void onError(cryptonote::ITransfersSubscription *object,
                       uint32_t height, std::error_code ec);

  // Interface cryptonote::ITransactionValidator

  virtual bool checkTransactionInputs(const cryptonote::Transaction &tx, cryptonote::BlockInfo &maxUsedBlock) override;
  virtual bool checkTransactionInputs(const cryptonote::Transaction &tx, cryptonote::BlockInfo &maxUsedBlock, cryptonote::BlockInfo &lastFailed) override;
  virtual bool haveSpentKeyImages(const cryptonote::Transaction &tx) override;
  virtual bool checkTransactionSize(size_t blobSize) override;

private:
  std::unique_ptr<cryptonote::NodeRpcProxy> m_node;
  std::string m_host;
  uint16_t m_port;

  bool m_isBlockchainSynced = false;
  std::unique_ptr<cryptonote::BlockchainSynchronizer> m_blockchainSync;
  std::unique_ptr<cryptonote::TransfersSyncronizer> m_transfersSync;
  std::unique_ptr<cryptonote::BlockchainExplorer> m_blockchainExplorer;
  std::unique_ptr<cryptonote::Blockchain> m_blockchain;

  std::unique_ptr<cryptonote::Currency> m_currency;

  bool m_connected = false;
  bool m_synced = false;

  std::mutex m_syncMutex;
  std::condition_variable m_syncCV;
};

} // namespace api