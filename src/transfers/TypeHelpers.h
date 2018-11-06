// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "ITransaction.h"
#include <functional>
#include <cstring>

namespace cryptonote {

inline bool operator==(const account_public_address_t &_v1, const account_public_address_t &_v2) {
  return memcmp(&_v1, &_v2, sizeof(account_public_address_t)) == 0;
}

}

namespace std {

template<>
struct hash < cryptonote::account_public_address_t > {
  size_t operator()(const cryptonote::account_public_address_t& val) const {
    size_t spend = *(reinterpret_cast<const size_t*>(&val.spendPublicKey));
    size_t view = *(reinterpret_cast<const size_t*>(&val.viewPublicKey));
    return spend ^ view;
  }
};

}
