// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <functional>
#include "cryptonote.h"
#include "P2pProtocolTypes.h"

namespace cryptonote {

  struct CryptoNoteConnectionContext;

  struct IP2pEndpoint {
    virtual void relay_notify_to_all(int command, const binary_array_t& data_buff, const net_connection_id* excludeConnection) = 0;
    virtual bool invoke_notify_to_peer(int command, const binary_array_t& req_buff, const cryptonote::CryptoNoteConnectionContext& context) = 0;
    virtual uint64_t get_connections_count()=0;
    virtual void for_each_connection(std::function<void(cryptonote::CryptoNoteConnectionContext&, peer_id_type_t)> f) = 0;
    // can be called from external threads
    virtual void externalRelayNotifyToAll(int command, const binary_array_t& data_buff) = 0;
  };

  struct p2p_endpoint_stub: public IP2pEndpoint {
    virtual void relay_notify_to_all(int command, const binary_array_t& data_buff, const net_connection_id* excludeConnection) override {}
    virtual bool invoke_notify_to_peer(int command, const binary_array_t& req_buff, const cryptonote::CryptoNoteConnectionContext& context) override { return true; }
    virtual void for_each_connection(std::function<void(cryptonote::CryptoNoteConnectionContext&, peer_id_type_t)> f) override {}
    virtual uint64_t get_connections_count() override { return 0; }   
    virtual void externalRelayNotifyToAll(int command, const binary_array_t& data_buff) override {}
  };
}
