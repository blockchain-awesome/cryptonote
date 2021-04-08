// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <list>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

#include "P2pProtocolTypes.h"
#include "CryptoNoteConfig.h"
#include "stream/cryptonote.h"

namespace cryptonote
{

  class ISerializer;
  /************************************************************************/
  /*                                                                      */
  /************************************************************************/
  class PeerlistManager
  {
    struct by_time
    {
    };
    struct by_id
    {
    };
    struct by_addr
    {
    };

    typedef boost::multi_index_container<
        peerlist_entry_t,
        boost::multi_index::indexed_by<
            // access by peerlist_entry::net_adress
            boost::multi_index::ordered_unique<boost::multi_index::tag<by_addr>, boost::multi_index::member<peerlist_entry_t, network_address_t, &peerlist_entry_t::adr>>,
            // sort by peerlist_entry::last_seen<
            boost::multi_index::ordered_non_unique<boost::multi_index::tag<by_time>, boost::multi_index::member<peerlist_entry_t, uint64_t, &peerlist_entry_t::last_seen>>>>
        peers_indexed;

  public:
    class Peerlist
    {
    public:
      Peerlist(peers_indexed &peers, size_t maxSize);
      size_t count() const;
      bool get(peerlist_entry_t &entry, size_t index) const;
      void trim();

      peers_indexed &m_peers;
      const size_t m_maxSize;
    };

    PeerlistManager();

    bool init(bool allow_local_ip);
    size_t get_white_peers_count() const { return m_peers_white.size(); }
    size_t get_gray_peers_count() const { return m_peers_gray.size(); }
    bool merge_peerlist(const std::list<peerlist_entry_t> &outer_bs);
    bool get_peerlist_head(std::list<peerlist_entry_t> &bs_head, uint32_t depth = cryptonote::P2P_DEFAULT_PEERS_IN_HANDSHAKE) const;
    bool get_peerlist_full(std::list<peerlist_entry_t> &pl_gray, std::list<peerlist_entry_t> &pl_white) const;
    bool get_white_peer_by_index(peerlist_entry_t &p, size_t i) const;
    bool get_gray_peer_by_index(peerlist_entry_t &p, size_t i) const;
    bool append_with_peer_white(const peerlist_entry_t &pr);
    bool append_with_peer_gray(const peerlist_entry_t &pr);
    bool set_peer_just_seen(peer_id_type_t peer, uint32_t ip, uint32_t port);
    bool set_peer_just_seen(peer_id_type_t peer, const network_address_t &addr);
    bool set_peer_unreachable(const peerlist_entry_t &pr);
    bool is_ip_allowed(uint32_t ip) const;
    void trim_white_peerlist();
    void trim_gray_peerlist();

    void serialize(ISerializer &s);

    Peerlist &getWhite();
    Peerlist &getGray();

    std::string m_config_folder;
    bool m_allow_local_ip;
    peers_indexed m_peers_gray;
    peers_indexed m_peers_white;
    Peerlist m_whitePeerlist;
    Peerlist m_grayPeerlist;
  };

  Reader &operator>>(Reader &i, PeerlistManager &v);

  Writer &operator<<(Writer &o, const PeerlistManager &v);

}
