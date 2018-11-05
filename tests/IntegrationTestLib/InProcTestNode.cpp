// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "InProcTestNode.h"

#include <future>

#include <common/StringTools.h>
#include <logging/ConsoleLogger.h>

#include "cryptonote/core/Core.h"
#include "cryptonote/core/Account.h"
#include "command_line/CoreConfig.h"
#include "cryptonote/core/Miner.h"
#include "cryptonote/protocol/handler.h"
#include "p2p/NetNode.h"
#include "InProcessNode/InProcessNode.h"

using namespace cryptonote;

#undef ERROR

namespace Tests {

namespace {
bool parse_peer_from_string(NetworkAddress &pe, const std::string &node_addr) {
  return ::Common::parseIpAddressAndPort(pe.ip, pe.port, node_addr);
}
}


InProcTestNode::InProcTestNode(const TestNodeConfiguration& cfg, const cryptonote::Currency& currency) : 
  m_cfg(cfg), m_currency(currency) {

  std::promise<std::string> initPromise;
  std::future<std::string> initFuture = initPromise.get_future();

  m_thread = std::thread(std::bind(&InProcTestNode::workerThread, this, std::ref(initPromise)));
  auto initError = initFuture.get();

  if (!initError.empty()) {
    m_thread.join();
    throw std::runtime_error(initError);
  }
}

InProcTestNode::~InProcTestNode() {
  if (m_thread.joinable()) {
    m_thread.join();
  }
}

void InProcTestNode::workerThread(std::promise<std::string>& initPromise) {

  System::Dispatcher dispatcher;

  Logging::ConsoleLogger log;

  Logging::LoggerRef logger(log, "InProcTestNode");

  try {

    core.reset(new cryptonote::core(m_currency, NULL, log));
    protocol.reset(new cryptonote::CryptoNoteProtocolHandler(m_currency, dispatcher, *core, NULL, log));
    p2pNode.reset(new cryptonote::NodeServer(dispatcher, *protocol, log));
    protocol->set_p2p_endpoint(p2pNode.get());
    core->set_cryptonote_protocol(protocol.get());

    cryptonote::NetNodeConfig p2pConfig;

    p2pConfig.setBindIp("127.0.0.1");
    p2pConfig.setBindPort(m_cfg.p2pPort);
    p2pConfig.setExternalPort(0);
    p2pConfig.setAllowLocalIp(false);
    p2pConfig.setHideMyPort(false);
    p2pConfig.setConfigFolder(m_cfg.dataDir);

    std::vector<NetworkAddress> exclusiveNodes;
    for (const auto& en : m_cfg.exclusiveNodes) {
      NetworkAddress na;
      parse_peer_from_string(na, en);
      exclusiveNodes.push_back(na);
    }

    p2pConfig.setExclusiveNodes(exclusiveNodes);

    if (!p2pNode->init(p2pConfig)) {
      throw std::runtime_error("Failed to init p2pNode");
    }

    cryptonote::MinerConfig emptyMiner;
    cryptonote::CoreConfig coreConfig;

    coreConfig.configFolder = m_cfg.dataDir;
    
    if (!core->init(emptyMiner, true)) {
      throw std::runtime_error("Core failed to initialize");
    }

    initPromise.set_value(std::string());

  } catch (std::exception& e) {
    logger(Logging::ERROR) << "Failed to initialize: " << e.what();
    initPromise.set_value(e.what());
    return;
  }

  try {
    p2pNode->run();
  } catch (std::exception& e) {
    logger(Logging::ERROR) << "exception in p2p::run: " << e.what();
  }

  core->deinit();
  p2pNode->deinit();
  core->set_cryptonote_protocol(NULL);
  protocol->set_p2p_endpoint(NULL);

  p2pNode.reset();
  protocol.reset();
  core.reset();
}

bool InProcTestNode::startMining(size_t threadsCount, const std::string &address) {
  assert(core.get());
  account_public_address_t addr;
  Account::parseAddress(address, addr);
  return core->get_miner().start(addr, threadsCount);
}

bool InProcTestNode::stopMining() {
  assert(core.get());
  return core->get_miner().stop();
}

bool InProcTestNode::stopDaemon() {
  if (!p2pNode.get()) {
    return false;
  }

  p2pNode->sendStopSignal();
  m_thread.join();
  return true;
}

bool InProcTestNode::getBlockTemplate(const std::string &minerAddress, cryptonote::block_t &blockTemplate, uint64_t &difficulty) {
  account_public_address_t addr;
  Account::parseAddress(minerAddress, addr);
  uint32_t height = 0;
  return core->get_block_template(blockTemplate, addr, difficulty, height, BinaryArray());
}

bool InProcTestNode::submitBlock(const std::string& block) {
  block_verification_context_t bvc = boost::value_initialized<block_verification_context_t>();
  core->handle_incoming_block_blob(Common::fromHex(block), bvc, true, true);
  return bvc.m_added_to_main_chain;
}

bool InProcTestNode::getTailBlockId(crypto::hash_t &tailBlockId) {
  tailBlockId = core->get_tail_id();
  return true;
}

bool InProcTestNode::makeINode(std::unique_ptr<cryptonote::INode> &node) {

  std::unique_ptr<InProcessNode> inprocNode(new cryptonote::InProcessNode(*core, *protocol));

  std::promise<std::error_code> p;
  auto future = p.get_future();

  inprocNode->init([&p](std::error_code ec) {
    std::promise<std::error_code> localPromise(std::move(p));
    localPromise.set_value(ec);
  });

  auto ec = future.get();

  if (!ec) {
    node = std::move(inprocNode);
    return true;
  }

  return false;
}

uint64_t InProcTestNode::getLocalHeight() {
  return core->get_current_blockchain_height();
}

}
