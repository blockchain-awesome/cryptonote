// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "crypto/crypto.h"
#include "cryptonote/core/key.h"

#include "SingleTransactionTestBase.h"

class test_derive_secret_key : public single_tx_test_base
{
public:
  static const size_t loop_count = 1000000;

  bool init()
  {
    if (!single_tx_test_base::init())
      return false;

    generate_key_derivation((const uint8_t*)&m_tx_pub_key, (const uint8_t*)&(m_bob.getAccountKeys().viewSecretKey), (uint8_t*)&m_key_derivation);
    m_spend_secret_key = m_bob.getAccountKeys().spendSecretKey;

    return true;
  }

  bool test()
  {
    cryptonote::key_pair_t in_ephemeral;
    crypto::derive_secret_key((const uint8_t*)&m_key_derivation, 0, (const uint8_t*)&m_spend_secret_key, (uint8_t*)&in_ephemeral.secretKey);
    return true;
  }

private:
  crypto::key_derivation_t m_key_derivation;
  crypto::secret_key_t m_spend_secret_key;
};
