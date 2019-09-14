// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <list>
#include <ostream>
#include <unordered_set>

#include <boost/uuid/uuid.hpp>
#include "common/StringTools.h"
#include "cryptonote/crypto/hash.h"

namespace cryptonote {

struct CryptoNoteConnectionContext {
  uint8_t version;
  boost::uuids::uuid m_connection_id;
  uint32_t m_remote_ip = 0;
  uint32_t m_remote_port = 0;
  bool m_is_income = false;
  time_t m_started = 0;

  enum state {
    state_befor_handshake = 0, //default state
    state_synchronizing,
    state_idle,
    state_normal,
    state_sync_required,
    state_pool_sync_required,
    state_shutdown
  };

  state m_state = state_befor_handshake;
  std::list<hash_t> m_needed_objects;
  std::unordered_set<hash_t> m_requested_objects;
  uint32_t m_remote_blockchain_height = 0;
  uint32_t m_last_response_height = 0;
};

inline std::string get_protocol_state_string(CryptoNoteConnectionContext::state s) {
  switch (s)  {
  case CryptoNoteConnectionContext::state_befor_handshake:
    return "state_befor_handshake";
  case CryptoNoteConnectionContext::state_synchronizing:
    return "state_synchronizing";
  case CryptoNoteConnectionContext::state_idle:
    return "state_idle";
  case CryptoNoteConnectionContext::state_normal:
    return "state_normal";
  case CryptoNoteConnectionContext::state_sync_required:
    return "state_sync_required";
  case CryptoNoteConnectionContext::state_pool_sync_required:
    return "state_pool_sync_required";
  case CryptoNoteConnectionContext::state_shutdown:
    return "state_shutdown";
  default:
    return "unknown";
  }
}

}

namespace std {
inline std::ostream& operator << (std::ostream& s, const cryptonote::CryptoNoteConnectionContext& context) {
  return s << "[" << Common::ipAddressToString(context.m_remote_ip) << ":" << 
    context.m_remote_port << (context.m_is_income ? " INC" : " OUT") << "] ";
}
}
