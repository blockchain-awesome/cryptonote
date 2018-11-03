// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>

#include "cryptonote/core/Account.h"
#include "cryptonote/core/key.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/Currency.h"
#include "cryptonote/core/TransactionExtra.h"

#include "crypto/crypto.h"

#include "logging/ConsoleLogger.h"

template<size_t a_ring_size>
class multi_tx_test_base
{
  static_assert(0 < a_ring_size, "ring_size must be greater than 0");

public:
  static const size_t ring_size = a_ring_size;
  static const size_t real_source_idx = ring_size / 2;

  bool init()
  {
    using namespace cryptonote;

    Currency currency = CurrencyBuilder(m_logger, os::appdata::path()).currency();

    std::vector<TransactionSourceEntry::OutputEntry> output_entries;
    for (uint32_t i = 0; i < ring_size; ++i)
    {
      m_miners[i].generate();

      if (!currency.constructMinerTx(0, 0, 0, 2, 0, m_miners[i].getAccountKeys().address, m_miner_txs[i]))
        return false;

      KeyOutput tx_out = boost::get<KeyOutput>(m_miner_txs[i].outputs[0].target);
      output_entries.push_back(std::make_pair(i, tx_out.key));
      m_public_keys[i] = tx_out.key;
      m_public_key_ptrs[i] = &m_public_keys[i];
    }

    m_source_amount = m_miner_txs[0].outputs[0].amount;

    TransactionSourceEntry source_entry;
    source_entry.amount = m_source_amount;
    source_entry.realTransactionPublicKey = getTransactionPublicKeyFromExtra(m_miner_txs[real_source_idx].extra);
    source_entry.realOutputIndexInTransaction = 0;
    source_entry.outputs.swap(output_entries);
    source_entry.realOutput = real_source_idx;

    m_sources.push_back(source_entry);

    return true;
  }

protected:
  cryptonote::AccountBase m_miners[ring_size];
  cryptonote::Transaction m_miner_txs[ring_size];
  uint64_t m_source_amount;
  Logging::ConsoleLogger m_logger;

  std::vector<cryptonote::TransactionSourceEntry> m_sources;
  crypto::public_key_t m_public_keys[ring_size];
  const crypto::public_key_t* m_public_key_ptrs[ring_size];
};
