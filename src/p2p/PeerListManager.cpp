// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PeerListManager.h"

#include <time.h>
#include <boost/foreach.hpp>
#include <system/Ipv4Address.h>

#include "serialization/SerializationOverloads.h"

using namespace cryptonote;

namespace cryptonote
{
  template <typename T, typename Indexes>
  bool serialize(boost::multi_index_container<T, Indexes> &value, Common::StringView name, ISerializer &s)
  {
    if (s.type() == ISerializer::INPUT)
    {
      readSequence<T>(std::inserter(value, value.end()), name, s);
    }
    else
    {
      writeSequence<T>(value.begin(), value.end(), name, s);
    }

    return true;
  }

  void serialize(network_address_t &na, ISerializer &s)
  {
    s(na.ip, "ip");
    s(na.port, "port");
  }

  void serialize(peerlist_entry_t &pe, ISerializer &s)
  {
    s(pe.adr, "adr");
    s(pe.id, "id");
    s(pe.last_seen, "last_seen");
  }

}

PeerlistManager::Peerlist::Peerlist(peers_indexed &peers, size_t maxSize) : m_peers(peers), m_maxSize(maxSize)
{
}

void PeerlistManager::serialize(ISerializer &s)
{
  const uint8_t currentVersion = 1;
  uint8_t version = currentVersion;

  s(version, "version");

  if (version != currentVersion)
  {
    return;
  }

  s(m_peers_white, "whitelist");
  s(m_peers_gray, "graylist");
}

size_t PeerlistManager::Peerlist::count() const
{
  return m_peers.size();
}

bool PeerlistManager::Peerlist::get(peerlist_entry_t &entry, size_t i) const
{
  if (i >= m_peers.size())
    return false;

  peers_indexed::index<by_time>::type &by_time_index = m_peers.get<by_time>();

  auto it = by_time_index.rbegin();
  std::advance(it, i);
  entry = *it;

  return true;
}

void PeerlistManager::Peerlist::trim()
{
  peers_indexed::index<by_time>::type &sorted_index = m_peers.get<by_time>();
  while (m_peers.size() > m_maxSize)
  {
    sorted_index.erase(sorted_index.begin());
  }
}

PeerlistManager::PeerlistManager() : m_whitePeerlist(m_peers_white, cryptonote::P2P_LOCAL_WHITE_PEERLIST_LIMIT),
                                     m_grayPeerlist(m_peers_gray, cryptonote::P2P_LOCAL_GRAY_PEERLIST_LIMIT) {}

//--------------------------------------------------------------------------------------------------
bool PeerlistManager::init(bool allow_local_ip)
{
  m_allow_local_ip = allow_local_ip;
  return true;
}

//--------------------------------------------------------------------------------------------------
void PeerlistManager::trim_white_peerlist()
{
  m_whitePeerlist.trim();
}
//--------------------------------------------------------------------------------------------------
void PeerlistManager::trim_gray_peerlist()
{
  m_grayPeerlist.trim();
}

//--------------------------------------------------------------------------------------------------
bool PeerlistManager::merge_peerlist(const std::list<peerlist_entry_t> &outer_bs)
{
  for (const peerlist_entry_t &be : outer_bs)
  {
    append_with_peer_gray(be);
  }

  // delete extra elements
  trim_gray_peerlist();
  return true;
}
//--------------------------------------------------------------------------------------------------

bool PeerlistManager::get_white_peer_by_index(peerlist_entry_t &p, size_t i) const
{
  return m_whitePeerlist.get(p, i);
}

//--------------------------------------------------------------------------------------------------

bool PeerlistManager::get_gray_peer_by_index(peerlist_entry_t &p, size_t i) const
{
  return m_grayPeerlist.get(p, i);
}

//--------------------------------------------------------------------------------------------------

bool PeerlistManager::is_ip_allowed(uint32_t ip) const
{
  System::Ipv4Address addr(networkToHost(ip));

  //never allow loopback ip
  if (addr.isLoopback())
  {
    return false;
  }

  if (!m_allow_local_ip && addr.isPrivate())
  {
    return false;
  }

  return true;
}
//--------------------------------------------------------------------------------------------------

bool PeerlistManager::get_peerlist_head(std::list<peerlist_entry_t> &bs_head, uint32_t depth) const
{
  const peers_indexed::index<by_time>::type &by_time_index = m_peers_white.get<by_time>();
  uint32_t cnt = 0;

  BOOST_REVERSE_FOREACH(const peers_indexed::value_type &vl, by_time_index)
  {
    if (!vl.last_seen)
      continue;
    bs_head.push_back(vl);
    if (cnt++ > depth)
      break;
  }
  return true;
}
//--------------------------------------------------------------------------------------------------

bool PeerlistManager::get_peerlist_full(std::list<peerlist_entry_t> &pl_gray, std::list<peerlist_entry_t> &pl_white) const
{
  const peers_indexed::index<by_time>::type &by_time_index_gr = m_peers_gray.get<by_time>();
  const peers_indexed::index<by_time>::type &by_time_index_wt = m_peers_white.get<by_time>();

  std::copy(by_time_index_gr.rbegin(), by_time_index_gr.rend(), std::back_inserter(pl_gray));
  std::copy(by_time_index_wt.rbegin(), by_time_index_wt.rend(), std::back_inserter(pl_white));

  return true;
}
//--------------------------------------------------------------------------------------------------

bool PeerlistManager::set_peer_just_seen(peer_id_type_t peer, uint32_t ip, uint32_t port)
{
  network_address_t addr;
  addr.ip = ip;
  addr.port = port;
  return set_peer_just_seen(peer, addr);
}
//--------------------------------------------------------------------------------------------------

bool PeerlistManager::set_peer_just_seen(peer_id_type_t peer, const network_address_t &addr)
{
  try
  {
    //find in white list
    peerlist_entry_t ple;
    ple.adr = addr;
    ple.id = peer;
    ple.last_seen = time(NULL);
    return append_with_peer_white(ple);
  }
  catch (std::exception &)
  {
  }

  return false;
}
//--------------------------------------------------------------------------------------------------

bool PeerlistManager::append_with_peer_white(const peerlist_entry_t &ple)
{
  try
  {
    if (!is_ip_allowed(ple.adr.ip))
      return true;

    //find in white list
    auto by_addr_it_wt = m_peers_white.get<by_addr>().find(ple.adr);
    if (by_addr_it_wt == m_peers_white.get<by_addr>().end())
    {
      //put new record into white list
      m_peers_white.insert(ple);
      trim_white_peerlist();
    }
    else
    {
      //update record in white list
      m_peers_white.replace(by_addr_it_wt, ple);
    }
    //remove from gray list, if need
    auto by_addr_it_gr = m_peers_gray.get<by_addr>().find(ple.adr);
    if (by_addr_it_gr != m_peers_gray.get<by_addr>().end())
    {
      m_peers_gray.erase(by_addr_it_gr);
    }
    return true;
  }
  catch (std::exception &)
  {
  }
  return false;
}
//--------------------------------------------------------------------------------------------------

bool PeerlistManager::append_with_peer_gray(const peerlist_entry_t &ple)
{
  try
  {
    if (!is_ip_allowed(ple.adr.ip))
      return true;

    //find in white list
    auto by_addr_it_wt = m_peers_white.get<by_addr>().find(ple.adr);
    if (by_addr_it_wt != m_peers_white.get<by_addr>().end())
      return true;

    //update gray list
    auto by_addr_it_gr = m_peers_gray.get<by_addr>().find(ple.adr);
    if (by_addr_it_gr == m_peers_gray.get<by_addr>().end())
    {
      //put new record into white list
      m_peers_gray.insert(ple);
      trim_gray_peerlist();
    }
    else
    {
      //update record in white list
      m_peers_gray.replace(by_addr_it_gr, ple);
    }
    return true;
  }
  catch (std::exception &)
  {
  }
  return false;
}
//--------------------------------------------------------------------------------------------------

PeerlistManager::Peerlist &PeerlistManager::getWhite()
{
  return m_whitePeerlist;
}

PeerlistManager::Peerlist &PeerlistManager::getGray()
{
  return m_grayPeerlist;
}

Reader &operator>>(Reader &i, network_address_t &v)
{
  i >> v.ip;
  i >> v.port;
  return i;
}
Writer &operator<<(Writer &o, const network_address_t &v)
{
  o << v.ip;
  o << v.port;
  return o;
}

Reader &operator>>(Reader &i, peerlist_entry_t &v)
{
  i >> v.adr;
  i >> v.id;
  i >> v.last_seen;
  return i;
}
Writer &operator<<(Writer &o, const peerlist_entry_t &v)
{
  o << v.adr;
  o << v.id;
  o << v.last_seen;
  return o;
}

template <typename T, typename Indexes>

Reader &operator>>(Reader &i, boost::multi_index_container<T, Indexes> &v)
{
  size_t size = 0;
  uint64_t s = 0;
  i >> s;
  size = s;

  while (size--)
  {
    peerlist_entry_t pe;
    i >> pe;
    v.insert(std::move(pe));
  }
  return i;
}

template <typename T, typename Indexes>

Writer &operator<<(Writer &o, const boost::multi_index_container<T, Indexes> &v)
{
  o << (uint64_t)v.size();
  for (auto &pe : v)
  {
    o << pe;
  }

  return o;
}

Reader &operator>>(Reader &i, PeerlistManager &v)
{
  const uint8_t currentVersion = 1;
  uint8_t version = currentVersion;

  i >> version;

  if (version != currentVersion)
  {
    return i;
  }

  i >> v.m_peers_white;
  i >> v.m_peers_gray;
  return i;
}

Writer &operator<<(Writer &o, const PeerlistManager &v)
{
  const uint8_t currentVersion = 1;
  uint8_t version = currentVersion;

  o << version;

  if (version != currentVersion)
  {
    return o;
  }

  o << v.m_peers_white;
  o << v.m_peers_gray;
  return o;
}