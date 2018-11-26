// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include "p2p/P2pProtocolTypes.h"

namespace cryptonote {

class NetNodeConfig {
public:
  NetNodeConfig();
  static void initOptions(boost::program_options::options_description& desc);
  bool init(const boost::program_options::variables_map& vm);

  std::string getP2pStateFilename() const;
  bool getTestnet() const;
  std::string getBindIp() const;
  uint16_t getBindPort() const;
  uint16_t getExternalPort() const;
  bool getAllowLocalIp() const;
  std::vector<peerlist_entry_t> getPeers() const;
  std::vector<network_address_t> getPriorityNodes() const;
  std::vector<network_address_t> getExclusiveNodes() const;
  std::vector<network_address_t> getSeedNodes() const;
  bool getHideMyPort() const;
  std::string getConfigFolder() const;

  void setP2pStateFilename(const std::string& filename);
  void setTestnet(bool isTestnet);
  void setBindIp(const std::string& ip);
  void setBindPort(uint16_t port);
  void setExternalPort(uint16_t port);
  void setAllowLocalIp(bool allow);
  void setPeers(const std::vector<peerlist_entry_t>& peerList);
  void setPriorityNodes(const std::vector<network_address_t>& addresses);
  void setExclusiveNodes(const std::vector<network_address_t>& addresses);
  void setSeedNodes(const std::vector<network_address_t>& addresses);
  void setHideMyPort(bool hide);
  void setConfigFolder(const std::string& folder);

private:
  std::string bindIp;
  uint16_t bindPort;
  uint16_t externalPort;
  bool allowLocalIp;
  std::vector<peerlist_entry_t> peers;
  std::vector<network_address_t> priorityNodes;
  std::vector<network_address_t> exclusiveNodes;
  std::vector<network_address_t> seedNodes;
  bool hideMyPort;
  std::string configFolder;
  std::string p2pStateFilename;
  bool testnet;
};

} //namespace nodetool
