// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <stdint.h>
#include <System/Dispatcher.h>
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "Rpc/HttpClient.h"

#include "TestNode.h"

using namespace cryptonote;

namespace Tests {
  class RPCTestNode : public TestNode {
  public:
    RPCTestNode(uint16_t port, System::Dispatcher& d);

    virtual bool startMining(size_t threadsCount, const std::string& address) override;
    virtual bool stopMining() override;
    virtual bool stopDaemon() override;
    virtual bool getBlockTemplate(const std::string& minerAddress, cryptonote::Block& blockTemplate, uint64_t& difficulty) override;
    virtual bool submitBlock(const std::string& block) override;
    virtual bool getTailBlockId(crypto::Hash& tailBlockId) override;
    virtual bool makeINode(std::unique_ptr<cryptonote::INode>& node) override;
    virtual uint64_t getLocalHeight() override;

    virtual ~RPCTestNode() { }

  private:

    uint16_t m_rpcPort;
    System::Dispatcher& m_dispatcher;
    cryptonote::HttpClient m_httpClient;
  };
}
