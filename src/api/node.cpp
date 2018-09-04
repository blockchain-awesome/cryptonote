#include <future>
#include <iostream>

#include "node.h"

namespace api
{
Node::Node(std::string &host, uint16_t port) : m_host(host), m_port(port)
{
  m_node.reset(new CryptoNote::NodeRpcProxy(host, port));
}

void Node::connectionStatusUpdated(bool connected)
{
  std::cout << "connectionStatusUpdated" << std::endl;
}

bool Node::init()
{
  std::promise<std::error_code> errorPromise;
  std::future<std::error_code> error = errorPromise.get_future();
  auto callback = [&errorPromise](std::error_code e) { errorPromise.set_value(e); };
  m_node->addObserver(static_cast<INodeRpcProxyObserver *>(this));
  m_node->init(callback);
  if (error.get())
  {
    return false;
  }
  return true;
}
} // namespace api