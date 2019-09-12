// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "crypto/crypto.h"
#include "cryptonote/core/key.h"

#include "SingleTransactionTestBase.h"

class test_generate_key_derivation : public single_tx_test_base
{
public:
  static const size_t loop_count = 1000;

  bool test()
  {
    key_derivation_t recv_derivation;
    generate_key_derivation((const uint8_t*)&m_tx_pub_key, (const uint8_t*)&(m_bob.getAccountKeys().viewSecretKey), (uint8_t*)&recv_derivation);
    return true;
  }
};
