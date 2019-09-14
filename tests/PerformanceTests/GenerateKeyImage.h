// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote/crypto/crypto.h"
#include "cryptonote/core/key.h"

#include "SingleTransactionTestBase.h"

class test_generate_key_image : public single_tx_test_base
{
public:
  static const size_t loop_count = 1000;

  bool init()
  {
    using namespace cryptonote;

    if (!single_tx_test_base::init())
      return false;

    account_keys_t bob_keys = m_bob.getAccountKeys();

    key_derivation_t recv_derivation;
    generate_key_derivation((const uint8_t*)&m_tx_pub_key, (const uint8_t*)&bob_keys.viewSecretKey, (uint8_t*)&recv_derivation);

    derive_public_key((const uint8_t*)&recv_derivation, 0, (const uint8_t*)&bob_keys.address.spendPublicKey, (uint8_t*)&m_in_ephemeral.publicKey);
    derive_secret_key((const uint8_t*)&recv_derivation, 0, (const uint8_t*)&bob_keys.spendSecretKey, (uint8_t*)&m_in_ephemeral.secretKey);

    return true;
  }

  bool test()
  {
    key_image_t ki;
    generate_key_image((const uint8_t *)&m_in_ephemeral.publicKey, (const uint8_t *)&m_in_ephemeral.secretKey, (uint8_t *)&ki);
    return true;
  }

private:
  cryptonote::key_pair_t m_in_ephemeral;
};
