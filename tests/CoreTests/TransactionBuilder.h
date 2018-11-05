// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote/core/Account.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/Currency.h"

class TransactionBuilder {
public:

  typedef std::vector<cryptonote::account_keys_t> KeysVector;
  typedef std::vector<crypto::signature_t> SignatureVector;
  typedef std::vector<SignatureVector> SignatureMultivector;

  struct MultisignatureSource {
    cryptonote::multi_signature_input_t input;
    KeysVector keys;
    crypto::public_key_t srcTxPubKey;
    size_t srcOutputIndex;
  };

  TransactionBuilder(const cryptonote::Currency& currency, uint64_t unlockTime = 0);

  // regenerate transaction keys
  TransactionBuilder& newTxKeys();
  TransactionBuilder& setTxKeys(const cryptonote::key_pair_t& txKeys);

  // inputs
  TransactionBuilder& setInput(const std::vector<cryptonote::transaction_source_entry_t>& sources, const cryptonote::account_keys_t& senderKeys);
  TransactionBuilder& addMultisignatureInput(const MultisignatureSource& source);

  // outputs
  TransactionBuilder& setOutput(const std::vector<cryptonote::transaction_destination_entry_t>& destinations);
  TransactionBuilder& addOutput(const cryptonote::transaction_destination_entry_t& dest);
  TransactionBuilder& addMultisignatureOut(uint64_t amount, const KeysVector& keys, uint32_t required);

  cryptonote::transaction_t build() const;

  std::vector<cryptonote::transaction_source_entry_t> m_sources;
  std::vector<cryptonote::transaction_destination_entry_t> m_destinations;

private:

  void fillInputs(cryptonote::transaction_t& tx, std::vector<cryptonote::key_pair_t>& contexts) const;
  void fillOutputs(cryptonote::transaction_t& tx) const;
  void signSources(const crypto::hash_t& prefixHash, const std::vector<cryptonote::key_pair_t>& contexts, cryptonote::transaction_t& tx) const;

  struct MultisignatureDestination {
    uint64_t amount;
    uint32_t requiredSignatures;
    KeysVector keys;
  };

  cryptonote::account_keys_t m_senderKeys;

  std::vector<MultisignatureSource> m_msigSources;
  std::vector<MultisignatureDestination> m_msigDestinations;

  size_t m_version;
  uint64_t m_unlockTime;
  cryptonote::key_pair_t m_txKey;
  const cryptonote::Currency& m_currency;
};
