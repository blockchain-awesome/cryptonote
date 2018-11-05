// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionUtils.h"

#include <unordered_set>

#include "crypto/crypto.h"
#include "cryptonote/core/Account.h"
#include "CryptoNoteFormatUtils.h"
#include "TransactionExtra.h"

using namespace crypto;

namespace cryptonote {

bool checkInputsKeyimagesDiff(const cryptonote::transaction_prefix_t& tx) {
  std::unordered_set<crypto::key_image_t> ki;
  for (const auto& in : tx.inputs) {
    if (in.type() == typeid(key_input_t)) {
      if (!ki.insert(boost::get<key_input_t>(in).keyImage).second)
        return false;
    }
  }
  return true;
}

// transaction_input_t helper functions

size_t getRequiredSignaturesCount(const transaction_input_t& in) {
  if (in.type() == typeid(key_input_t)) {
    return boost::get<key_input_t>(in).outputIndexes.size();
  }
  if (in.type() == typeid(multi_signature_input_t)) {
    return boost::get<multi_signature_input_t>(in).signatureCount;
  }
  return 0;
}

uint64_t getTransactionInputAmount(const transaction_input_t& in) {
  if (in.type() == typeid(key_input_t)) {
    return boost::get<key_input_t>(in).amount;
  }
  if (in.type() == typeid(multi_signature_input_t)) {
    return boost::get<multi_signature_input_t>(in).amount;
  }
  return 0;
}

TransactionTypes::input_type_t getTransactionInputType(const transaction_input_t& in) {
  if (in.type() == typeid(key_input_t)) {
    return TransactionTypes::input_type_t::Key;
  }
  if (in.type() == typeid(multi_signature_input_t)) {
    return TransactionTypes::input_type_t::Multisignature;
  }
  if (in.type() == typeid(base_input_t)) {
    return TransactionTypes::input_type_t::Generating;
  }
  return TransactionTypes::input_type_t::Invalid;
}

const transaction_input_t& getInputChecked(const cryptonote::transaction_prefix_t& transaction, size_t index) {
  if (transaction.inputs.size() <= index) {
    throw std::runtime_error("transaction_t input index out of range");
  }
  return transaction.inputs[index];
}

const transaction_input_t& getInputChecked(const cryptonote::transaction_prefix_t& transaction, size_t index, TransactionTypes::input_type_t type) {
  const auto& input = getInputChecked(transaction, index);
  if (getTransactionInputType(input) != type) {
    throw std::runtime_error("Unexpected transaction input type");
  }
  return input;
}

// transaction_output_t helper functions

TransactionTypes::output_type_t getTransactionOutputType(const transaction_output_target_t& out) {
  if (out.type() == typeid(key_output_t)) {
    return TransactionTypes::output_type_t::Key;
  }
  if (out.type() == typeid(multi_signature_output_t)) {
    return TransactionTypes::output_type_t::Multisignature;
  }
  return TransactionTypes::output_type_t::Invalid;
}

const transaction_output_t& getOutputChecked(const cryptonote::transaction_prefix_t& transaction, size_t index) {
  if (transaction.outputs.size() <= index) {
    throw std::runtime_error("transaction_t output index out of range");
  }
  return transaction.outputs[index];
}

const transaction_output_t& getOutputChecked(const cryptonote::transaction_prefix_t& transaction, size_t index, TransactionTypes::output_type_t type) {
  const auto& output = getOutputChecked(transaction, index);
  if (getTransactionOutputType(output.target) != type) {
    throw std::runtime_error("Unexpected transaction output target type");
  }
  return output;
}

bool isOutToKey(const crypto::public_key_t& spendPublicKey, const crypto::public_key_t& outKey, const crypto::key_derivation_t& derivation, size_t keyIndex) {
  crypto::public_key_t pk;
  derive_public_key(derivation, keyIndex, spendPublicKey, pk);
  return pk == outKey;
}

bool findOutputsToAccount(const cryptonote::transaction_prefix_t& transaction, const account_public_address_t& addr,
                          const secret_key_t& viewSecretKey, std::vector<uint32_t>& out, uint64_t& amount) {
  account_keys_t keys;
  keys.address = addr;
  // only view secret key is used, spend key is not needed
  keys.viewSecretKey = viewSecretKey;

  crypto::public_key_t txPubKey = getTransactionPublicKeyFromExtra(transaction.extra);

  amount = 0;
  size_t keyIndex = 0;
  uint32_t outputIndex = 0;

  crypto::key_derivation_t derivation;
  generate_key_derivation(txPubKey, keys.viewSecretKey, derivation);

  for (const transaction_output_t& o : transaction.outputs) {
    assert(o.target.type() == typeid(key_output_t) || o.target.type() == typeid(multi_signature_output_t));
    if (o.target.type() == typeid(key_output_t)) {
      if (is_out_to_acc(keys, boost::get<key_output_t>(o.target), derivation, keyIndex)) {
        out.push_back(outputIndex);
        amount += o.amount;
      }
      ++keyIndex;
    } else if (o.target.type() == typeid(multi_signature_output_t)) {
      const auto& target = boost::get<multi_signature_output_t>(o.target);
      for (const auto& key : target.keys) {
        if (isOutToKey(keys.address.spendPublicKey, key, derivation, static_cast<size_t>(outputIndex))) {
          out.push_back(outputIndex);
        }
        ++keyIndex;
      }
    }
    ++outputIndex;
  }

  return true;
}

}
