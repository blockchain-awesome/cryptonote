// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string.h>
#include <tuple>
#include <boost/uuid/uuid.hpp>
#include "types.h"
#include "common/str.h"


namespace cryptonote
{
  inline bool operator < (const network_address_t& a, const network_address_t& b) {
    return std::tie(a.ip, a.port) < std::tie(b.ip, b.port);
  }

  inline bool operator == (const network_address_t& a, const network_address_t& b) {
    return memcmp(&a, &b, sizeof(a)) == 0;
  }

  inline std::ostream& operator << (std::ostream& s, const network_address_t& na) {
    return s << ::string::IPv4::to(na.ip) << std::string(":") << std::to_string(na.port);   
  }

  inline uint32_t hostToNetwork(uint32_t n) {
    return (n << 24) | (n & 0xff00) << 8 | (n & 0xff0000) >> 8 | (n >> 24);
  }

  inline uint32_t networkToHost(uint32_t n) {
    return hostToNetwork(n); // the same
  }

}
