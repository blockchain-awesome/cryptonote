// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "TestNode.h"
#include "NetworkConfiguration.h"

#include <future>
#include <memory>
#include <thread>

#include <system/Dispatcher.h>


namespace cryptonote {
class core;
class CryptoNoteProtocolHandler;
class NodeServer;
class Currency;
}

namespace Tests {

class InProcTestNode : public TestNode {
public:
  InProcTestNode(const TestNodeConfiguration& cfg, const cryptonote::Currency& currency);
  ~InProcTestNode();

  virtual bool startMining(size_t threadsCount, const std::string &address) override;
  virtual bool stopMining() override;
  virtual bool stopDaemon() override;
  virtual bool getBlockTemplate(const std::string &minerAddress, cryptonote::block_t &blockTemplate, uint64_t &difficulty) override;
  virtual bool submitBlock(const std::string& block) override;
  virtual bool getTailBlockId(crypto::Hash &tailBlockId) override;
  virtual bool makeINode(std::unique_ptr<cryptonote::INode>& node) override;
  virtual uint64_t getLocalHeight() override;

private:

  void workerThread(std::promise<std::string>& initPromise);

  std::unique_ptr<cryptonote::core> core;
  std::unique_ptr<cryptonote::CryptoNoteProtocolHandler> protocol;
  std::unique_ptr<cryptonote::NodeServer> p2pNode;

  std::thread m_thread;
  const cryptonote::Currency& m_currency;
  TestNodeConfiguration m_cfg;
};

}
