// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <functional>
#include <list>

#include <logging/LoggerRef.h>
#include <system/ContextGroup.h>
#include <system/Dispatcher.h>
#include <system/Event.h>
#include <system/TcpListener.h>
#include <system/Timer.h>

#include "IP2pNodeInternal.h"
#include "IStreamSerializable.h"
#include "command_line/NetNodeConfig.h"
#include "P2pInterfaces.h"
#include "command_line/P2pNodeConfig.h"
#include "P2pProtocolDefinitions.h"
#include "PeerListManager.h"

namespace cryptonote {

class P2pContext;
class P2pConnectionProxy;

class P2pNode : 
  public IP2pNode, 
  public IStreamSerializable,
  IP2pNodeInternal {

public:

  P2pNode(
    const P2pNodeConfig& cfg,
    System::Dispatcher& dispatcher, 
    Logging::ILogger& log, 
    const crypto::hash_t& genesisHash, 
    peer_id_type_t peerId);

  ~P2pNode();
  
  // IP2pNode
  virtual std::unique_ptr<IP2pConnection> receiveConnection() override;
  virtual void stop() override;

  // IStreamSerializable
  virtual void save(std::ostream& os) override;
  virtual void load(std::istream& in) override;

  // P2pNode
  void start();
  void serialize(ISerializer& s);

private:
  typedef std::unique_ptr<P2pContext> ContextPtr;
  typedef std::list<ContextPtr> ContextList;

  Logging::LoggerRef logger;
  bool m_stopRequested;
  const P2pNodeConfig m_cfg;
  const peer_id_type_t m_myPeerId;
  const crypto::hash_t m_genesisHash;
  const CORE_SYNC_DATA m_genesisPayload;

  System::Dispatcher& m_dispatcher;
  System::ContextGroup workingContextGroup;
  System::TcpListener m_listener;
  System::Timer m_connectorTimer;
  PeerlistManager m_peerlist;
  ContextList m_contexts;
  System::Event m_queueEvent;
  std::deque<std::unique_ptr<IP2pConnection>> m_connectionQueue;

  // IP2pNodeInternal
  virtual const CORE_SYNC_DATA& getGenesisPayload() const override;
  virtual std::list<peerlist_entry_t> getLocalPeerList() const override;
  virtual basic_node_data getNodeData() const override;
  virtual peer_id_type_t getPeerId() const override;

  virtual void handleNodeData(const basic_node_data& node, P2pContext& ctx) override;
  virtual bool handleRemotePeerList(const std::list<peerlist_entry_t>& peerlist, time_t local_time) override;
  virtual void tryPing(P2pContext& ctx) override;

  // spawns
  void acceptLoop();
  void connectorLoop();

  // connection related
  void connectPeers();
  void connectPeerList(const std::vector<network_address_t>& peers);
  bool isPeerConnected(const network_address_t& address);
  bool isPeerUsed(const peerlist_entry_t& peer);
  ContextPtr tryToConnectPeer(const network_address_t& address);
  bool fetchPeerList(ContextPtr connection);

  // making and processing connections
  size_t getOutgoingConnectionsCount() const;
  void makeExpectedConnectionsCount(const PeerlistManager::Peerlist& peerlist, size_t connectionsCount);
  bool makeNewConnectionFromPeerlist(const PeerlistManager::Peerlist& peerlist);
  void preprocessIncomingConnection(ContextPtr ctx);
  void enqueueConnection(std::unique_ptr<P2pConnectionProxy> proxy);
  std::unique_ptr<P2pConnectionProxy> createProxy(ContextPtr ctx);
};

}
