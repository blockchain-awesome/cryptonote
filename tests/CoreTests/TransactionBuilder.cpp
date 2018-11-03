// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionBuilder.h"
#include "cryptonote/core/TransactionExtra.h"
#include "cryptonote/core/CryptoNoteTools.h"

using namespace cryptonote;
using namespace crypto;
using namespace Common;

TransactionBuilder::TransactionBuilder(const cryptonote::Currency& currency, uint64_t unlockTime)
  : m_currency(currency), m_version(cryptonote::CURRENT_TRANSACTION_VERSION), m_unlockTime(unlockTime), 
  // m_txKey(generateKeyPair())
  m_txKey(Key::generate())
  {}

TransactionBuilder& TransactionBuilder::newTxKeys() {
  // m_txKey = generateKeyPair();
  m_txKey = Key::generate();
  return *this;
}

TransactionBuilder& TransactionBuilder::setTxKeys(const cryptonote::KeyPair& txKeys) {
  m_txKey = txKeys;
  return *this;
}

TransactionBuilder& TransactionBuilder::setInput(const std::vector<cryptonote::TransactionSourceEntry>& sources, const cryptonote::AccountKeys& senderKeys) {
  m_sources = sources;
  m_senderKeys = senderKeys;
  return *this;
}

TransactionBuilder& TransactionBuilder::addMultisignatureInput(const MultisignatureSource& source) {
  m_msigSources.push_back(source);
  return *this;
}

TransactionBuilder& TransactionBuilder::setOutput(const std::vector<cryptonote::TransactionDestinationEntry>& destinations) {
  m_destinations = destinations;
  return *this;
}

TransactionBuilder& TransactionBuilder::addOutput(const cryptonote::TransactionDestinationEntry& dest) {
  m_destinations.push_back(dest);
  return *this;
}

TransactionBuilder& TransactionBuilder::addMultisignatureOut(uint64_t amount, const KeysVector& keys, uint32_t required) {

  MultisignatureDestination dst;

  dst.amount = amount;
  dst.keys = keys;
  dst.requiredSignatures = required;

  m_msigDestinations.push_back(dst);

  return *this;
}

Transaction TransactionBuilder::build() const {
  crypto::Hash prefixHash;

  Transaction tx;
  addTransactionPublicKeyToExtra(tx.extra, m_txKey.publicKey);

  tx.version = static_cast<uint8_t>(m_version);
  tx.unlockTime = m_unlockTime;

  std::vector<cryptonote::KeyPair> contexts;

  fillInputs(tx, contexts);
  fillOutputs(tx);

  getObjectHash(*static_cast<TransactionPrefix*>(&tx), prefixHash);

  signSources(prefixHash, contexts, tx);

  return tx;
}

void TransactionBuilder::fillInputs(Transaction& tx, std::vector<cryptonote::KeyPair>& contexts) const {
  for (const TransactionSourceEntry& src_entr : m_sources) {
    contexts.push_back(KeyPair());
    KeyPair& in_ephemeral = contexts.back();
    crypto::KeyImage img;
    generate_key_image_helper(m_senderKeys, src_entr.realTransactionPublicKey, src_entr.realOutputIndexInTransaction, in_ephemeral, img);

    // put key image into tx input
    KeyInput input_to_key;
    input_to_key.amount = src_entr.amount;
    input_to_key.keyImage = img;

    // fill outputs array and use relative offsets
    for (const TransactionSourceEntry::OutputEntry& out_entry : src_entr.outputs) {
      input_to_key.outputIndexes.push_back(out_entry.first);
    }

    input_to_key.outputIndexes = absolute_output_offsets_to_relative(input_to_key.outputIndexes);
    tx.inputs.push_back(input_to_key);
  }

  for (const auto& msrc : m_msigSources) {
    tx.inputs.push_back(msrc.input);
  }
}

void TransactionBuilder::fillOutputs(Transaction& tx) const {
  size_t output_index = 0;
  
  for(const auto& dst_entr : m_destinations) {
    crypto::key_derivation_t derivation;
    crypto::public_key_t out_eph_public_key;
    crypto::generate_key_derivation(dst_entr.addr.viewPublicKey, m_txKey.secretKey, derivation);
    crypto::derive_public_key(derivation, output_index, dst_entr.addr.spendPublicKey, out_eph_public_key);

    TransactionOutput out;
    out.amount = dst_entr.amount;
    KeyOutput tk;
    tk.key = out_eph_public_key;
    out.target = tk;
    tx.outputs.push_back(out);
    output_index++;
  }

  for (const auto& mdst : m_msigDestinations) {   
    TransactionOutput out;
    MultisignatureOutput target;

    target.requiredSignatureCount = mdst.requiredSignatures;

    for (const auto& key : mdst.keys) {
      crypto::key_derivation_t derivation;
      crypto::public_key_t ephemeralPublicKey;
      crypto::generate_key_derivation(key.address.viewPublicKey, m_txKey.secretKey, derivation);
      crypto::derive_public_key(derivation, output_index, key.address.spendPublicKey, ephemeralPublicKey);
      target.keys.push_back(ephemeralPublicKey);
    }
    out.amount = mdst.amount;
    out.target = target;
    tx.outputs.push_back(out);
    output_index++;
  }
}


void TransactionBuilder::signSources(const crypto::Hash& prefixHash, const std::vector<cryptonote::KeyPair>& contexts, Transaction& tx) const {
  
  tx.signatures.clear();

  size_t i = 0;

  // sign TransactionInputToKey sources
  for (const auto& src_entr : m_sources) {
    std::vector<const crypto::public_key_t*> keys_ptrs;

    for (const auto& o : src_entr.outputs) {
      keys_ptrs.push_back(&o.second);
    }

    tx.signatures.push_back(std::vector<crypto::signature_t>());
    std::vector<crypto::signature_t>& sigs = tx.signatures.back();
    sigs.resize(src_entr.outputs.size());
    generate_ring_signature(prefixHash, boost::get<KeyInput>(tx.inputs[i]).keyImage, keys_ptrs, contexts[i].secretKey, src_entr.realOutput, sigs.data());
    i++;
  }

  // sign multisignature source
  for (const auto& msrc : m_msigSources) {
    tx.signatures.resize(tx.signatures.size() + 1);
    auto& outsigs = tx.signatures.back();

    for (const auto& key : msrc.keys) {
      crypto::key_derivation_t derivation;
      crypto::public_key_t ephemeralPublicKey;
      crypto::secret_key_t ephemeralSecretKey;

      crypto::generate_key_derivation(msrc.srcTxPubKey, key.viewSecretKey, derivation);
      crypto::derive_public_key(derivation, msrc.srcOutputIndex, key.address.spendPublicKey, ephemeralPublicKey);
      crypto::derive_secret_key(derivation, msrc.srcOutputIndex, key.spendSecretKey, ephemeralSecretKey);

      crypto::signature_t sig;
      crypto::generate_signature(prefixHash, ephemeralPublicKey, ephemeralSecretKey, sig);
      outsigs.push_back(sig);
    }
  }
}
