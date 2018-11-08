// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ITransaction.h"

#include <numeric>
#include <system_error>

#include "cryptonote/core/key.h"
#include "cryptonote/core/TransactionApiExtra.h"
#include "TransactionUtils.h"
#include "cryptonote/core/CryptoNoteTools.h"

using namespace crypto;

namespace cryptonote {

class TransactionPrefixImpl : public ITransactionReader {
public:
  TransactionPrefixImpl();
  TransactionPrefixImpl(const transaction_prefix_t& prefix, const hash_t& transactionHash);

  virtual ~TransactionPrefixImpl() { }

  virtual hash_t getTransactionHash() const override;
  virtual hash_t getTransactionPrefixHash() const override;
  virtual public_key_t getTransactionPublicKey() const override;
  virtual uint64_t getUnlockTime() const override;

  // extra
  virtual bool getPaymentId(hash_t& paymentId) const override;
  virtual bool getExtraNonce(BinaryArray& nonce) const override;
  virtual BinaryArray getExtra() const override;

  // inputs
  virtual size_t getInputCount() const override;
  virtual uint64_t getInputTotalAmount() const override;
  virtual TransactionTypes::input_type_t getInputType(size_t index) const override;
  virtual void getInput(size_t index, key_input_t& input) const override;
  virtual void getInput(size_t index, multi_signature_input_t& input) const override;

  // outputs
  virtual size_t getOutputCount() const override;
  virtual uint64_t getOutputTotalAmount() const override;
  virtual TransactionTypes::output_type_t getOutputType(size_t index) const override;
  virtual void getOutput(size_t index, key_output_t& output, uint64_t& amount) const override;
  virtual void getOutput(size_t index, multi_signature_output_t& output, uint64_t& amount) const override;

  // signatures
  virtual size_t getRequiredSignaturesCount(size_t inputIndex) const override;
  virtual bool findOutputsToAccount(const account_public_address_t& addr, const secret_key_t& viewSecretKey, std::vector<uint32_t>& outs, uint64_t& outputAmount) const override;

  // various checks
  virtual bool validateInputs() const override;
  virtual bool validateOutputs() const override;
  virtual bool validateSignatures() const override;

  // serialized transaction
  virtual BinaryArray getTransactionData() const override;

  virtual bool getTransactionSecretKey(secret_key_t& key) const override;

private:
  transaction_prefix_t m_txPrefix;
  TransactionExtra m_extra;
  hash_t m_txHash;
};

TransactionPrefixImpl::TransactionPrefixImpl() {
}

TransactionPrefixImpl::TransactionPrefixImpl(const transaction_prefix_t& prefix, const hash_t& transactionHash) {
  m_extra.parse(prefix.extra);

  m_txPrefix = prefix;
  m_txHash = transactionHash;
}

hash_t TransactionPrefixImpl::getTransactionHash() const {
  return m_txHash;
}

hash_t TransactionPrefixImpl::getTransactionPrefixHash() const {
  return getObjectHash(m_txPrefix);
}

public_key_t TransactionPrefixImpl::getTransactionPublicKey() const {
  crypto::public_key_t pk(NULL_PUBLIC_KEY);
  m_extra.getPublicKey(pk);
  return pk;
}

uint64_t TransactionPrefixImpl::getUnlockTime() const {
  return m_txPrefix.unlockTime;
}

bool TransactionPrefixImpl::getPaymentId(hash_t& hash) const {
  BinaryArray nonce;

  if (getExtraNonce(nonce)) {
    crypto::hash_t paymentId;
    if (getPaymentIdFromTransactionExtraNonce(nonce, paymentId)) {
      hash = reinterpret_cast<const hash_t&>(paymentId);
      return true;
    }
  }

  return false;
}

bool TransactionPrefixImpl::getExtraNonce(BinaryArray& nonce) const {
  transaction_extra_nonce_t extraNonce;

  if (m_extra.get(extraNonce)) {
    nonce = extraNonce.nonce;
    return true;
  }

  return false;
}

BinaryArray TransactionPrefixImpl::getExtra() const {
  return m_txPrefix.extra;
}

size_t TransactionPrefixImpl::getInputCount() const {
  return m_txPrefix.inputs.size();
}

uint64_t TransactionPrefixImpl::getInputTotalAmount() const {
  return std::accumulate(m_txPrefix.inputs.begin(), m_txPrefix.inputs.end(), 0ULL, [](uint64_t val, const transaction_input_t& in) {
    return val + getTransactionInputAmount(in); });
}

TransactionTypes::input_type_t TransactionPrefixImpl::getInputType(size_t index) const {
  return getTransactionInputType(getInputChecked(m_txPrefix, index));
}

void TransactionPrefixImpl::getInput(size_t index, key_input_t& input) const {
  input = boost::get<key_input_t>(getInputChecked(m_txPrefix, index, TransactionTypes::input_type_t::Key));
}

void TransactionPrefixImpl::getInput(size_t index, multi_signature_input_t& input) const {
  input = boost::get<multi_signature_input_t>(getInputChecked(m_txPrefix, index, TransactionTypes::input_type_t::Multisignature));
}

size_t TransactionPrefixImpl::getOutputCount() const {
  return m_txPrefix.outputs.size();
}

uint64_t TransactionPrefixImpl::getOutputTotalAmount() const {
  return std::accumulate(m_txPrefix.outputs.begin(), m_txPrefix.outputs.end(), 0ULL, [](uint64_t val, const transaction_output_t& out) {
    return val + out.amount; });
}

TransactionTypes::output_type_t TransactionPrefixImpl::getOutputType(size_t index) const {
  return getTransactionOutputType(getOutputChecked(m_txPrefix, index).target);
}

void TransactionPrefixImpl::getOutput(size_t index, key_output_t& output, uint64_t& amount) const {
  const auto& out = getOutputChecked(m_txPrefix, index, TransactionTypes::output_type_t::Key);
  output = boost::get<key_output_t>(out.target);
  amount = out.amount;
}

void TransactionPrefixImpl::getOutput(size_t index, multi_signature_output_t& output, uint64_t& amount) const {
  const auto& out = getOutputChecked(m_txPrefix, index, TransactionTypes::output_type_t::Multisignature);
  output = boost::get<multi_signature_output_t>(out.target);
  amount = out.amount;
}

size_t TransactionPrefixImpl::getRequiredSignaturesCount(size_t inputIndex) const {
  return ::cryptonote::getRequiredSignaturesCount(getInputChecked(m_txPrefix, inputIndex));
}

bool TransactionPrefixImpl::findOutputsToAccount(const account_public_address_t& addr, const secret_key_t& viewSecretKey, std::vector<uint32_t>& outs, uint64_t& outputAmount) const {
  return ::cryptonote::findOutputsToAccount(m_txPrefix, addr, viewSecretKey, outs, outputAmount);
}

bool TransactionPrefixImpl::validateInputs() const {
  return check_inputs_types_supported(m_txPrefix) &&
          check_inputs_overflow(m_txPrefix) &&
          checkInputsKeyimagesDiff(m_txPrefix) &&
          checkMultisignatureInputsDiff(m_txPrefix);
}

bool TransactionPrefixImpl::validateOutputs() const {
  return check_outs_valid(m_txPrefix) &&
          check_outs_overflow(m_txPrefix);
}

bool TransactionPrefixImpl::validateSignatures() const {
  throw std::system_error(std::make_error_code(std::errc::function_not_supported), "Validating signatures is not supported for transaction prefix");
}

BinaryArray TransactionPrefixImpl::getTransactionData() const {
  return toBinaryArray(m_txPrefix);
}

bool TransactionPrefixImpl::getTransactionSecretKey(secret_key_t& key) const {
  return false;
}


std::unique_ptr<ITransactionReader> createTransactionPrefix(const transaction_prefix_t& prefix, const hash_t& transactionHash) {
  return std::unique_ptr<ITransactionReader> (new TransactionPrefixImpl(prefix, transactionHash));
}

std::unique_ptr<ITransactionReader> createTransactionPrefix(const transaction_t& fullTransaction) {
  return std::unique_ptr<ITransactionReader> (new TransactionPrefixImpl(fullTransaction, getObjectHash(fullTransaction)));
}

}
