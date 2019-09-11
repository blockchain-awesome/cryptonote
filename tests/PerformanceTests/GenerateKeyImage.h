// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "crypto/crypto.h"
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

    crypto::key_derivation_t recv_derivation;
    generate_key_derivation((const uint8_t*)&m_tx_pub_key, (const uint8_t*)&bob_keys.viewSecretKey, (uint8_t*)&recv_derivation);

    crypto::derive_public_key(recv_derivation, 0, bob_keys.address.spendPublicKey, m_in_ephemeral.publicKey);
    crypto::derive_secret_key(recv_derivation, 0, bob_keys.spendSecretKey, m_in_ephemeral.secretKey);

    return true;
  }

  bool test()
  {
    crypto::key_image_t ki;
    crypto::generate_key_image(m_in_ephemeral.publicKey, m_in_ephemeral.secretKey, ki);
    return true;
  }

private:
  cryptonote::key_pair_t m_in_ephemeral;
};
