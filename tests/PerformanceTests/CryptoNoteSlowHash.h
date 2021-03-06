// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "common/hex.h"
#include "cryptonote/crypto/crypto.h"
#include "cryptonote/core/key.h"

class test_cn_slow_hash {
public:
  static const size_t loop_count = 10;

#pragma pack(push, 1)
  struct data_t {
    char data[13];
  };
#pragma pack(pop)

  static_assert(13 == sizeof(data_t), "Invalid structure size");

  bool init() {
    if (!hex::from("63617665617420656d70746f72", &m_data, sizeof(m_data)) || 13 != sizeof(m_data)) {
      return false;
    }

    if (!hex::from("bbec2cacf69866a8e740380fe7b818fc78f8571221742d729d9d02d7f8989b87", &m_expected_hash, sizeof(m_expected_hash)) || 32 != sizeof(m_expected_hash)) {
      return false;
    }

    return true;
  }

  bool test() {
    hash_t hash;
    crypto::cn_slow_hash(&m_data, sizeof(m_data), (char *) &hash, 0, 0);
    return hash == m_expected_hash;
  }

private:
  data_t m_data;
  hash_t m_expected_hash;
};
