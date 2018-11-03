// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote/core/Account.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/Currency.h"

class TransactionBuilder {
public:

  typedef std::vector<cryptonote::AccountKeys> KeysVector;
  typedef std::vector<crypto::signature_t> SignatureVector;
  typedef std::vector<SignatureVector> SignatureMultivector;

  struct MultisignatureSource {
    cryptonote::MultisignatureInput input;
    KeysVector keys;
    crypto::public_key_t srcTxPubKey;
    size_t srcOutputIndex;
  };

  TransactionBuilder(const cryptonote::Currency& currency, uint64_t unlockTime = 0);

  // regenerate transaction keys
  TransactionBuilder& newTxKeys();
  TransactionBuilder& setTxKeys(const cryptonote::KeyPair& txKeys);

  // inputs
  TransactionBuilder& setInput(const std::vector<cryptonote::TransactionSourceEntry>& sources, const cryptonote::AccountKeys& senderKeys);
  TransactionBuilder& addMultisignatureInput(const MultisignatureSource& source);

  // outputs
  TransactionBuilder& setOutput(const std::vector<cryptonote::TransactionDestinationEntry>& destinations);
  TransactionBuilder& addOutput(const cryptonote::TransactionDestinationEntry& dest);
  TransactionBuilder& addMultisignatureOut(uint64_t amount, const KeysVector& keys, uint32_t required);

  cryptonote::Transaction build() const;

  std::vector<cryptonote::TransactionSourceEntry> m_sources;
  std::vector<cryptonote::TransactionDestinationEntry> m_destinations;

private:

  void fillInputs(cryptonote::Transaction& tx, std::vector<cryptonote::KeyPair>& contexts) const;
  void fillOutputs(cryptonote::Transaction& tx) const;
  void signSources(const crypto::Hash& prefixHash, const std::vector<cryptonote::KeyPair>& contexts, cryptonote::Transaction& tx) const;

  struct MultisignatureDestination {
    uint64_t amount;
    uint32_t requiredSignatures;
    KeysVector keys;
  };

  cryptonote::AccountKeys m_senderKeys;

  std::vector<MultisignatureSource> m_msigSources;
  std::vector<MultisignatureDestination> m_msigDestinations;

  size_t m_version;
  uint64_t m_unlockTime;
  cryptonote::KeyPair m_txKey;
  const cryptonote::Currency& m_currency;
};
