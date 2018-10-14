// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote_core/Account.h"
#include "cryptonote_core/CryptoNoteBasic.h"
#include "cryptonote_core/CryptoNoteFormatUtils.h"

#include <Logging/LoggerGroup.h>

class single_tx_test_base
{
public:
  bool init()
  {
    using namespace cryptonote;

    Currency currency = CurrencyBuilder(m_nullLog).currency();
    m_bob.generate();

    if (!currency.constructMinerTx(0, 0, 0, 2, 0, m_bob.getAccountKeys().address, m_tx))
      return false;

    m_tx_pub_key = getTransactionPublicKeyFromExtra(m_tx.extra);
    return true;
  }

protected:

  Logging::LoggerGroup m_nullLog;
  cryptonote::AccountBase m_bob;
  cryptonote::Transaction m_tx;
  crypto::PublicKey m_tx_pub_key;
};
