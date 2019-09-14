// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NetNodeConfig.h"

#include <boost/utility/value_init.hpp>

#include "common/os.h"
#include "command_line/options.h"
#include "common/StringTools.h"
#include "cryptonote/crypto/crypto.h"
#include "CryptoNoteConfig.h"

using namespace command_line;

namespace cryptonote {
namespace {

bool parsePeerFromString(network_address_t& pe, const std::string& node_addr) {
  return Common::parseIpAddressAndPort(pe.ip, pe.port, node_addr);
}

bool parsePeersAndAddToContainer(const boost::program_options::variables_map& vm,
    const arg_descriptor<std::vector<std::string>>& arg, std::vector<network_address_t>& container)
{
  std::vector<std::string> peers = get_arg(vm, arg);

  for(const std::string& str: peers) {
    network_address_t na = boost::value_initialized<network_address_t>();
    if (!parsePeerFromString(na, str)) {
      return false;
    }
    container.push_back(na);
  }

  return true;
}

} //namespace

void NetNodeConfig::initOptions(boost::program_options::options_description& desc) {
  command_line::init();
  add_arg(desc, arg_p2p_bind_ip);
  add_arg(desc, arg_p2p_bind_port);
  add_arg(desc, arg_p2p_external_port);
  add_arg(desc, arg_p2p_allow_local_ip);
  add_arg(desc, arg_p2p_add_peer);
  add_arg(desc, arg_p2p_add_priority_node);
  add_arg(desc, arg_p2p_add_exclusive_node);
  add_arg(desc, arg_p2p_seed_node);
  add_arg(desc, arg_p2p_hide_my_port);
}

NetNodeConfig::NetNodeConfig():bindIp(""), bindPort(0),
 externalPort(0), allowLocalIp(false), hideMyPort(false), configFolder("")
{
}

bool NetNodeConfig::init(const boost::program_options::variables_map& vm)
{
  
  if (vm.count(arg_p2p_bind_ip.name) != 0 && (!vm[arg_p2p_bind_ip.name].defaulted() || bindIp.empty())) {
    bindIp = get_arg(vm, arg_p2p_bind_ip);
  }

  bindPort = config::get().net.p2p_port;
  if (vm.count(arg_p2p_bind_port.name) != 0 && (!vm[arg_p2p_bind_port.name].defaulted())) {
    bindPort = get_arg(vm, arg_p2p_bind_port);
  }

  if (vm.count(arg_p2p_external_port.name) != 0 && (!vm[arg_p2p_external_port.name].defaulted() || externalPort == 0)) {
    externalPort = get_arg(vm, arg_p2p_external_port);
  }

  if (vm.count(arg_p2p_allow_local_ip.name) != 0 && (!vm[arg_p2p_allow_local_ip.name].defaulted() || !allowLocalIp)) {
    allowLocalIp = get_arg(vm, arg_p2p_allow_local_ip);
  }
  configFolder == os::appdata::path();
  if (vm.count(arg_data_dir.name) != 0 && (!vm[arg_data_dir.name].defaulted())) {
    configFolder = get_arg(vm, arg_data_dir);
  }

  p2pStateFilename = config::get().filenames.p2p;

  if (has_arg(vm, arg_p2p_add_peer)) {
    std::vector<std::string> perrs = get_arg(vm, arg_p2p_add_peer);
    for(const std::string& pr_str: perrs) {
      peerlist_entry_t pe = boost::value_initialized<peerlist_entry_t>();
      pe.id = crypto::rand<uint64_t>();
      if (!parsePeerFromString(pe.adr, pr_str)) {
        return false;
      }

      peers.push_back(pe);
    }
  }

  if (has_arg(vm,arg_p2p_add_exclusive_node)) {
    if (!parsePeersAndAddToContainer(vm, arg_p2p_add_exclusive_node, exclusiveNodes))
      return false;
  }

  if (has_arg(vm, arg_p2p_add_priority_node)) {
    if (!parsePeersAndAddToContainer(vm, arg_p2p_add_priority_node, priorityNodes))
      return false;
  }

  if (has_arg(vm, arg_p2p_seed_node)) {
    if (!parsePeersAndAddToContainer(vm, arg_p2p_seed_node, seedNodes))
      return false;
  }

  if (has_arg(vm, arg_p2p_hide_my_port)) {
    hideMyPort = true;
  }

  return true;
}

std::string NetNodeConfig::getP2pStateFilename() const {
  return p2pStateFilename;
}

std::string NetNodeConfig::getBindIp() const {
  return bindIp;
}

uint16_t NetNodeConfig::getBindPort() const {
  return bindPort;
}

uint16_t NetNodeConfig::getExternalPort() const {
  return externalPort;
}

bool NetNodeConfig::getAllowLocalIp() const {
  return allowLocalIp;
}

std::vector<peerlist_entry_t> NetNodeConfig::getPeers() const {
  return peers;
}

std::vector<network_address_t> NetNodeConfig::getPriorityNodes() const {
  return priorityNodes;
}

std::vector<network_address_t> NetNodeConfig::getExclusiveNodes() const {
  return exclusiveNodes;
}

std::vector<network_address_t> NetNodeConfig::getSeedNodes() const {
  return seedNodes;
}

bool NetNodeConfig::getHideMyPort() const {
  return hideMyPort;
}

std::string NetNodeConfig::getConfigFolder() const {
  return configFolder;
}

void NetNodeConfig::setP2pStateFilename(const std::string& filename) {
  p2pStateFilename = filename;
}

void NetNodeConfig::setBindIp(const std::string& ip) {
  bindIp = ip;
}

void NetNodeConfig::setBindPort(uint16_t port) {
  bindPort = port;
}

void NetNodeConfig::setExternalPort(uint16_t port) {
  externalPort = port;
}

void NetNodeConfig::setAllowLocalIp(bool allow) {
  allowLocalIp = allow;
}

void NetNodeConfig::setPeers(const std::vector<peerlist_entry_t>& peerList) {
  peers = peerList;
}

void NetNodeConfig::setPriorityNodes(const std::vector<network_address_t>& addresses) {
  priorityNodes = addresses;
}

void NetNodeConfig::setExclusiveNodes(const std::vector<network_address_t>& addresses) {
  exclusiveNodes = addresses;
}

void NetNodeConfig::setSeedNodes(const std::vector<network_address_t>& addresses) {
  seedNodes = addresses;
}

void NetNodeConfig::setHideMyPort(bool hide) {
  hideMyPort = hide;
}

void NetNodeConfig::setConfigFolder(const std::string& folder) {
  configFolder = folder;
}


} //namespace nodetool
