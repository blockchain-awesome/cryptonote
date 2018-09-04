#pragma once

#include <string>
#include <cstdint>

#include "NodeRpcProxy/NodeRpcProxy.h"

namespace api
{
class Node : public CryptoNote::INodeRpcProxyObserver
{

public:
  Node(std::string &host, uint16_t port);
  bool init();


  // Interface CryptoNote::INodeRpcProxyObserver
    virtual void connectionStatusUpdated(bool connected) override;


private:
  std::unique_ptr<CryptoNote::NodeRpcProxy> m_node;
  std::string m_host;
  uint16_t m_port;
};

} // namespace api