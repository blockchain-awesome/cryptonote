// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NetNodeConfig.h"

#include <boost/utility/value_init.hpp>

#include "common/os.h"
#include "command_line/options.h"
#include "common/StringTools.h"
#include "crypto/crypto.h"
#include "CryptoNoteConfig.h"

using namespace command_line;

namespace cryptonote {
namespace {

const arg_descriptor<std::string> arg_p2p_bind_ip        = {"p2p-bind-ip", "Interface for p2p network protocol", "0.0.0.0"};
const arg_descriptor<uint16_t>    arg_p2p_bind_port      = {"p2p-bind-port", "Port for p2p network protocol", config::get().net.p2p_port};
const arg_descriptor<uint16_t>    arg_p2p_external_port = { "p2p-external-port", "External port for p2p network protocol (if port forwarding used with NAT)", 0 };
const arg_descriptor<bool>        arg_p2p_allow_local_ip = {"allow-local-ip", "Allow local ip add to peer list, mostly in debug purposes"};
const arg_descriptor<std::vector<std::string> > arg_p2p_add_peer   = {"add-peer", "Manually add peer to local peerlist"};
const arg_descriptor<std::vector<std::string> > arg_p2p_add_priority_node   = {"add-priority-node", "Specify list of peers to connect to and attempt to keep the connection open"};
const arg_descriptor<std::vector<std::string> > arg_p2p_add_exclusive_node   = {"add-exclusive-node", "Specify list of peers to connect to only."
      " If this option is given the options add-priority-node and seed-node are ignored"};
const arg_descriptor<std::vector<std::string> > arg_p2p_seed_node   = {"seed-node", "Connect to a node to retrieve peer addresses, and disconnect"};
const arg_descriptor<bool> arg_p2p_hide_my_port   =    {"hide-my-port", "Do not announce yourself as peerlist candidate", false, true};

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
 externalPort(0), allowLocalIp(false), hideMyPort(false), configFolder(os::appdata::path()), testnet(false)
{
}

bool NetNodeConfig::init(const boost::program_options::variables_map& vm)
{
  if (vm.count(arg_p2p_bind_ip.name) != 0 && (!vm[arg_p2p_bind_ip.name].defaulted() || bindIp.empty())) {
    bindIp = get_arg(vm, arg_p2p_bind_ip);
  }

  if (vm.count(arg_p2p_bind_port.name) != 0 && (!vm[arg_p2p_bind_port.name].defaulted() || bindPort == 0)) {
    bindPort = get_arg(vm, arg_p2p_bind_port);
  }

  if (vm.count(arg_p2p_external_port.name) != 0 && (!vm[arg_p2p_external_port.name].defaulted() || externalPort == 0)) {
    externalPort = get_arg(vm, arg_p2p_external_port);
  }

  if (vm.count(arg_p2p_allow_local_ip.name) != 0 && (!vm[arg_p2p_allow_local_ip.name].defaulted() || !allowLocalIp)) {
    allowLocalIp = get_arg(vm, arg_p2p_allow_local_ip);
  }

  if (vm.count(arg_data_dir.name) != 0 && (!vm[arg_data_dir.name].defaulted() || configFolder == os::appdata::path())) {
    configFolder = get_arg(vm, arg_data_dir);
  }

  p2pStateFilename = cryptonote::parameters::P2P_NET_DATA_FILENAME;

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

void NetNodeConfig::setTestnet(bool isTestnet) {
  testnet = isTestnet;
}

std::string NetNodeConfig::getP2pStateFilename() const {
  if (testnet) {
    return "testnet_" + p2pStateFilename;
  }

  return p2pStateFilename;
}

bool NetNodeConfig::getTestnet() const {
  return testnet;
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
