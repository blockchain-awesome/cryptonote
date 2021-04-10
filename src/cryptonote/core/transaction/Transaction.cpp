// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ITransaction.h"
#include "TransactionApiExtra.h"
#include "TransactionUtils.h"
#include "transaction.h"
#include "account.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "CryptoNoteConfig.h"
#include "stream/transaction.h"
#include "cryptonote/structures/array.hpp"

#include <boost/optional.hpp>
#include <numeric>
#include <unordered_set>

using namespace crypto;

namespace {

  using namespace cryptonote;

  void derivePublicKey(const account_public_address_t& to, const secret_key_t& txKey, size_t outputIndex, public_key_t& ephemeralKey) {
    key_derivation_t derivation;
    generate_key_derivation((const uint8_t*)&to.viewPublicKey, (const uint8_t*)&txKey, (uint8_t*)&derivation);
    derive_public_key((const uint8_t *)&derivation, outputIndex, (const uint8_t *)&to.spendPublicKey, (uint8_t *)&ephemeralKey);
  }

}

namespace cryptonote {

  using namespace crypto;

  ////////////////////////////////////////////////////////////////////////
  // class transaction_t declaration
  ////////////////////////////////////////////////////////////////////////

  class TransactionImpl : public ITransaction {
  public:
    TransactionImpl();
    TransactionImpl(const binary_array_t& txblob);
    TransactionImpl(const cryptonote::transaction_t& tx);
  
    // ITransactionReader
    virtual hash_t getTransactionHash() const override;
    virtual hash_t getTransactionPrefixHash() const override;
    virtual public_key_t getTransactionPublicKey() const override;
    virtual uint64_t getUnlockTime() const override;
    virtual bool getPaymentId(hash_t& hash) const override;
    virtual bool getExtraNonce(binary_array_t& nonce) const override;
    virtual binary_array_t getExtra() const override;

    // inputs
    virtual size_t getInputCount() const override;
    virtual uint64_t getInputTotalAmount() const override;
    virtual input_type_t getInputType(size_t index) const override;
    virtual void getInput(size_t index, key_input_t& input) const override;
    virtual void getInput(size_t index, multi_signature_input_t& input) const override;

    // outputs
    virtual size_t getOutputCount() const override;
    virtual uint64_t getOutputTotalAmount() const override;
    virtual output_type_t getOutputType(size_t index) const override;
    virtual void getOutput(size_t index, key_output_t& output, uint64_t& amount) const override;
    virtual void getOutput(size_t index, multi_signature_output_t& output, uint64_t& amount) const override;

    virtual size_t getRequiredSignaturesCount(size_t index) const override;
    virtual bool findOutputsToAccount(const account_public_address_t& addr, const secret_key_t& viewSecretKey, std::vector<uint32_t>& outs, uint64_t& outputAmount) const override;

    // various checks
    virtual bool validateInputs() const override;
    virtual bool validateOutputs() const override;
    virtual bool validateSignatures() const override;

    // get serialized transaction
    virtual binary_array_t getTransactionData() const override;

    // ITransactionWriter

    virtual void setUnlockTime(uint64_t unlockTime) override;
    virtual void setPaymentId(const hash_t& hash) override;
    virtual void setExtraNonce(const binary_array_t& nonce) override;
    virtual void appendExtra(const binary_array_t& extraData) override;

    // Inputs/Outputs 
    virtual size_t addInput(const key_input_t& input) override;
    virtual size_t addInput(const multi_signature_input_t& input) override;
    virtual size_t addInput(const account_keys_t& senderKeys, const input_key_info_t& info, key_pair_t& ephKeys) override;

    virtual size_t addOutput(uint64_t amount, const account_public_address_t& to) override;
    virtual size_t addOutput(uint64_t amount, const std::vector<account_public_address_t>& to, uint32_t requiredSignatures) override;
    virtual size_t addOutput(uint64_t amount, const key_output_t& out) override;
    virtual size_t addOutput(uint64_t amount, const multi_signature_output_t& out) override;

    virtual void signInputKey(size_t input, const input_key_info_t& info, const key_pair_t& ephKeys) override;
    virtual void signInputMultisignature(size_t input, const public_key_t& sourceTransactionKey, size_t outputIndex, const account_keys_t& accountKeys) override;
    virtual void signInputMultisignature(size_t input, const key_pair_t& ephemeralKeys) override;


    // secret key
    virtual bool getTransactionSecretKey(secret_key_t& key) const override;
    virtual void setTransactionSecretKey(const secret_key_t& key) override;

  private:

    void invalidateHash();

    std::vector<signature_t>& getSignatures(size_t input);

    const secret_key_t& txSecretKey() const {
      if (!secretKey) {
        throw std::runtime_error("Operation requires transaction secret key");
      }
      return *secretKey;
    }

    void checkIfSigning() const {
      if (!transaction.signatures.empty()) {
        throw std::runtime_error("Cannot perform requested operation, since it will invalidate transaction signatures");
      }
    }

    cryptonote::transaction_t transaction;
    boost::optional<secret_key_t> secretKey;
    mutable boost::optional<hash_t> transactionHash;
    TransactionExtra extra;
  };


  ////////////////////////////////////////////////////////////////////////
  // class transaction_t implementation
  ////////////////////////////////////////////////////////////////////////

  std::unique_ptr<ITransaction> createTransaction() {
    return std::unique_ptr<ITransaction>(new TransactionImpl());
  }

  std::unique_ptr<ITransaction> createTransaction(const binary_array_t& transactionBlob) {
    return std::unique_ptr<ITransaction>(new TransactionImpl(transactionBlob));
  }

  std::unique_ptr<ITransaction> createTransaction(const cryptonote::transaction_t& tx) {
    return std::unique_ptr<ITransaction>(new TransactionImpl(tx));
  }

  TransactionImpl::TransactionImpl() {   
    // cryptonote::key_pair_t txKeys(cryptonote::generateKeyPair());
    key_pair_t txKeys = Key::generate();

    transaction_extra_public_key_t pk = { txKeys.publicKey };
    extra.set(pk);

    transaction.version = config::get().transaction.version.major;
    transaction.unlockTime = 0;
    transaction.extra = extra.serialize();

    secretKey = txKeys.secretKey;
  }

  TransactionImpl::TransactionImpl(const binary_array_t& ba) {
    if (!BinaryArray::from(transaction, ba)) {
      throw std::runtime_error("Invalid transaction data");
    }
    
    extra.parse(transaction.extra);
    transactionHash = BinaryArray::hash(ba); // avoid serialization if we already have blob
  }

  TransactionImpl::TransactionImpl(const cryptonote::transaction_t& tx) : transaction(tx) {
    extra.parse(transaction.extra);
  }

  void TransactionImpl::invalidateHash() {
    if (transactionHash.is_initialized()) {
      transactionHash = decltype(transactionHash)();
    }
  }

  hash_t TransactionImpl::getTransactionHash() const {
    if (!transactionHash.is_initialized()) {
      transactionHash = BinaryArray::objectHash(transaction);
    }

    return transactionHash.get();   
  }

  hash_t TransactionImpl::getTransactionPrefixHash() const {
    return BinaryArray::objectHash(*static_cast<const transaction_prefix_t*>(&transaction));
  }

  public_key_t TransactionImpl::getTransactionPublicKey() const {
    public_key_t pk(NULL_PUBLIC_KEY);
    extra.getPublicKey(pk);
    return pk;
  }

  uint64_t TransactionImpl::getUnlockTime() const {
    return transaction.unlockTime;
  }

  void TransactionImpl::setUnlockTime(uint64_t unlockTime) {
    checkIfSigning();
    transaction.unlockTime = unlockTime;
    invalidateHash();
  }

  bool TransactionImpl::getTransactionSecretKey(secret_key_t& key) const {
    if (!secretKey) {
      return false;
    }
    key = reinterpret_cast<const secret_key_t&>(secretKey.get());
    return true;
  }

  void TransactionImpl::setTransactionSecretKey(const secret_key_t& key) {
    const auto& sk = reinterpret_cast<const secret_key_t&>(key);
    public_key_t pk;
    public_key_t txPubKey;

    secret_key_to_public_key((const uint8_t*)&sk, (uint8_t*)&pk);
    extra.getPublicKey(txPubKey);

    if (txPubKey != pk) {
      throw std::runtime_error("Secret transaction key does not match public key");
    }

    secretKey = key;
  }

  size_t TransactionImpl::addInput(const key_input_t& input) {
    checkIfSigning();
    transaction.inputs.emplace_back(input);
    invalidateHash();
    return transaction.inputs.size() - 1;
  }

  size_t TransactionImpl::addInput(const account_keys_t& senderKeys, const input_key_info_t& info, key_pair_t& ephKeys) {
    checkIfSigning();
    key_input_t input;
    input.amount = info.amount;

    generate_key_image_helper(
      senderKeys,
      info.realOutput.transactionPublicKey,
      info.realOutput.outputInTransaction,
      ephKeys,
      input.keyImage);

    // fill outputs array and use relative offsets
    for (const auto& out : info.outputs) {
      input.outputIndexes.push_back(out.outputIndex);
    }

    input.outputIndexes = absolute_output_offsets_to_relative(input.outputIndexes);
    return addInput(input);
  }

  size_t TransactionImpl::addInput(const multi_signature_input_t& input) {
    checkIfSigning();
    transaction.inputs.push_back(input);
    invalidateHash();
    return transaction.inputs.size() - 1;
  }

  size_t TransactionImpl::addOutput(uint64_t amount, const account_public_address_t& to) {
    checkIfSigning();

    key_output_t outKey;
    derivePublicKey(to, txSecretKey(), transaction.outputs.size(), outKey.key);
    transaction_output_t out = { amount, outKey };
    transaction.outputs.emplace_back(out);
    invalidateHash();

    return transaction.outputs.size() - 1;
  }

  size_t TransactionImpl::addOutput(uint64_t amount, const std::vector<account_public_address_t>& to, uint32_t requiredSignatures) {
    checkIfSigning();

    const auto& txKey = txSecretKey();
    size_t outputIndex = transaction.outputs.size();
    multi_signature_output_t outMsig;
    outMsig.requiredSignatureCount = requiredSignatures;
    outMsig.keys.resize(to.size());
    
    for (size_t i = 0; i < to.size(); ++i) {
      derivePublicKey(to[i], txKey, outputIndex, outMsig.keys[i]);
    }

    transaction_output_t out = { amount, outMsig };
    transaction.outputs.emplace_back(out);
    invalidateHash();

    return outputIndex;
  }

  size_t TransactionImpl::addOutput(uint64_t amount, const key_output_t& out) {
    checkIfSigning();
    size_t outputIndex = transaction.outputs.size();
    transaction_output_t realOut = { amount, out };
    transaction.outputs.emplace_back(realOut);
    invalidateHash();
    return outputIndex;
  }

  size_t TransactionImpl::addOutput(uint64_t amount, const multi_signature_output_t& out) {
    checkIfSigning();
    size_t outputIndex = transaction.outputs.size();
    transaction_output_t realOut = { amount, out };
    transaction.outputs.emplace_back(realOut);
    invalidateHash();
    return outputIndex;
  }

  void TransactionImpl::signInputKey(size_t index, const input_key_info_t& info, const key_pair_t& ephKeys) {
    const auto& input = boost::get<key_input_t>(getInputChecked(transaction, index, input_type_t::Key));
    hash_t prefixHash = getTransactionPrefixHash();

    std::vector<signature_t> signatures;
    std::vector<const public_key_t*> keysPtrs;

    for (const auto& o : info.outputs) {
      keysPtrs.push_back(reinterpret_cast<const public_key_t*>(&o.targetKey));
    }

    signatures.resize(keysPtrs.size());

    generate_ring_signature(
      (const uint8_t*)&(prefixHash),
      (const uint8_t*)&(input.keyImage),
      (const uint8_t *const *)keysPtrs.data(),
      keysPtrs.size(),
      (const uint8_t*)&(ephKeys.secretKey),
      info.realOutput.transactionIndex,
      (uint8_t *)signatures.data());

    getSignatures(index) = signatures;
    invalidateHash();
  }

  void TransactionImpl::signInputMultisignature(size_t index, const public_key_t& sourceTransactionKey, size_t outputIndex, const account_keys_t& accountKeys) {
    key_derivation_t derivation;
    public_key_t ephemeralPublicKey;
    secret_key_t ephemeralSecretKey;

    generate_key_derivation(
      (const uint8_t*)&sourceTransactionKey,
      (const uint8_t*)&accountKeys.viewSecretKey,
      (uint8_t*)&derivation);

    derive_public_key((const uint8_t *)&derivation, outputIndex,
      (const uint8_t *)&(accountKeys.address.spendPublicKey), (uint8_t *)&ephemeralPublicKey);
    derive_secret_key((const uint8_t *)&derivation, outputIndex,
      (const uint8_t *)&(accountKeys.spendSecretKey), (uint8_t *)&ephemeralSecretKey);

    signature_t signature;
    auto txPrefixHash = getTransactionPrefixHash();

    generate_signature((const uint8_t *)&txPrefixHash,
      (const uint8_t *)&ephemeralPublicKey, (const uint8_t *)&ephemeralSecretKey, (uint8_t *)&signature);

    getSignatures(index).push_back(signature);
    invalidateHash();
  }

  void TransactionImpl::signInputMultisignature(size_t index, const key_pair_t& ephemeralKeys) {
    signature_t signature;
    auto txPrefixHash = getTransactionPrefixHash();

    generate_signature((const uint8_t *)&txPrefixHash, (const uint8_t *)&ephemeralKeys.publicKey, (const uint8_t *)&ephemeralKeys.secretKey, (uint8_t *)&signature);

    getSignatures(index).push_back(signature);
    invalidateHash();
  }

  std::vector<signature_t>& TransactionImpl::getSignatures(size_t input) {
    // update signatures container size if needed
    if (transaction.signatures.size() < transaction.inputs.size()) {
      transaction.signatures.resize(transaction.inputs.size());
    }
    // check range
    if (input >= transaction.signatures.size()) {
      throw std::runtime_error("Invalid input index");
    }

    return transaction.signatures[input];
  }

  binary_array_t TransactionImpl::getTransactionData() const {
    return BinaryArray::to(transaction);
  }

  void TransactionImpl::setPaymentId(const hash_t& hash) {
    checkIfSigning();
    binary_array_t paymentIdBlob;
    setPaymentIdToTransactionExtraNonce(paymentIdBlob, reinterpret_cast<const hash_t&>(hash));
    setExtraNonce(paymentIdBlob);
  }

  bool TransactionImpl::getPaymentId(hash_t& hash) const {
    binary_array_t nonce;
    if (getExtraNonce(nonce)) {
      hash_t paymentId;
      if (getPaymentIdFromTransactionExtraNonce(nonce, paymentId)) {
        hash = reinterpret_cast<const hash_t&>(paymentId);
        return true;
      }
    }
    return false;
  }

  void TransactionImpl::setExtraNonce(const binary_array_t& nonce) {
    checkIfSigning();
    transaction_extra_nonce_t extraNonce = { nonce };
    extra.set(extraNonce);
    transaction.extra = extra.serialize();
    invalidateHash();
  }

  void TransactionImpl::appendExtra(const binary_array_t& extraData) {
    checkIfSigning();
    transaction.extra.insert(
      transaction.extra.end(), extraData.begin(), extraData.end());
  }

  bool TransactionImpl::getExtraNonce(binary_array_t& nonce) const {
    transaction_extra_nonce_t extraNonce;
    if (extra.get(extraNonce)) {
      nonce = extraNonce.nonce;
      return true;
    }
    return false;
  }

  binary_array_t TransactionImpl::getExtra() const {
    return transaction.extra;
  }

  size_t TransactionImpl::getInputCount() const {
    return transaction.inputs.size();
  }

  uint64_t TransactionImpl::getInputTotalAmount() const {
    return std::accumulate(transaction.inputs.begin(), transaction.inputs.end(), 0ULL, [](uint64_t val, const transaction_input_t& in) {
      return val + getTransactionInputAmount(in); });
  }

  input_type_t TransactionImpl::getInputType(size_t index) const {
    return getTransactionInputType(getInputChecked(transaction, index));
  }

  void TransactionImpl::getInput(size_t index, key_input_t& input) const {
    input = boost::get<key_input_t>(getInputChecked(transaction, index, input_type_t::Key));
  }

  void TransactionImpl::getInput(size_t index, multi_signature_input_t& input) const {
    input = boost::get<multi_signature_input_t>(getInputChecked(transaction, index, input_type_t::Multisignature));
  }

  size_t TransactionImpl::getOutputCount() const {
    return transaction.outputs.size();
  }

  uint64_t TransactionImpl::getOutputTotalAmount() const {
    return std::accumulate(transaction.outputs.begin(), transaction.outputs.end(), 0ULL, [](uint64_t val, const transaction_output_t& out) {
      return val + out.amount; });
  }

  output_type_t TransactionImpl::getOutputType(size_t index) const {
    return getTransactionOutputType(getOutputChecked(transaction, index).target);
  }

  void TransactionImpl::getOutput(size_t index, key_output_t& output, uint64_t& amount) const {
    const auto& out = getOutputChecked(transaction, index, output_type_t::Key);
    output = boost::get<key_output_t>(out.target);
    amount = out.amount;
  }

  void TransactionImpl::getOutput(size_t index, multi_signature_output_t& output, uint64_t& amount) const {
    const auto& out = getOutputChecked(transaction, index, output_type_t::Multisignature);
    output = boost::get<multi_signature_output_t>(out.target);
    amount = out.amount;
  }

  bool TransactionImpl::findOutputsToAccount(const account_public_address_t& addr, const secret_key_t& viewSecretKey, std::vector<uint32_t>& out, uint64_t& amount) const {
    return ::cryptonote::findOutputsToAccount(transaction, addr, viewSecretKey, out, amount);
  }

  size_t TransactionImpl::getRequiredSignaturesCount(size_t index) const {
    return ::getRequiredSignaturesCount(getInputChecked(transaction, index));
  }

  bool TransactionImpl::validateInputs() const {
    return
      check_inputs_types_supported(transaction) &&
      check_inputs_overflow(transaction) &&
      checkInputsKeyimagesDiff(transaction) &&
      checkMultisignatureInputsDiff(transaction);
  }

  bool TransactionImpl::validateOutputs() const {
    return
      check_outs_valid(transaction) &&
      check_outs_overflow(transaction);
  }

  bool TransactionImpl::validateSignatures() const {
    if (transaction.signatures.size() < transaction.inputs.size()) {
      return false;
    }

    for (size_t i = 0; i < transaction.inputs.size(); ++i) {
      if (getRequiredSignaturesCount(i) > transaction.signatures[i].size()) {
        return false;
      }
    }

    return true;
  }

  // Old transaction.cpp code

  uint64_t Transaction::getInputAmount()
  {
    uint64_t amount = 0;
    for (auto item : m_transaction.inputs)
    {
      amount += getInputAmount(item);
    }
    return amount;
  }

  uint64_t Transaction::getOutputAmount()
  {
    uint64_t amount = 0;
    for (auto item : m_transaction.outputs)
    {
      amount += item.amount;
    }
    return amount;
  }

  uint64_t Transaction::getInputAmount(const transaction_input_t &in)
  {
    if (in.type() == typeid(key_input_t))
    {
      return boost::get<key_input_t>(in).amount;
    }
    if (in.type() == typeid(multi_signature_input_t))
    {
      return boost::get<multi_signature_input_t>(in).amount;
    }
    return 0;
  }

  uint64_t Transaction::getMixin(uint64_t mixin)
  {
    for (auto in : m_transaction.inputs)
    {
      if (in.type() != typeid(key_input_t))
      {
        continue;
      }
      key_input_t ki = boost::get<key_input_t>(in);

      uint64_t size = ki.outputIndexes.size();
      if (mixin < size)
      {
        mixin = size;
      }
    }
    return mixin;
  }

  void Transaction::getInputDetails(std::vector<transaction_input_details_t> inputs)
  {

    for (auto in : m_transaction.inputs)
    {
      transaction_input_details_t tid;
      if (in.type() == typeid(key_input_t))
      {
        transaction_input_to_key_details_t details;
        key_input_t kinput = boost::get<key_input_t>(in);

        std::vector<std::pair<hash_t, size_t>> outputReferences;
        outputReferences.reserve(kinput.outputIndexes.size());
        details.outputIndexes = kinput.outputIndexes;

        std::vector<uint32_t> res = kinput.outputIndexes;
        for (size_t i = 1; i < res.size(); i++)
        {
          res[i] += res[i - 1];
        }
        std::vector<uint32_t> globalIndexes = res;

        details.mixin = kinput.outputIndexes.size();
        // details.output.number = outputReferences.back().second;
        // details.output.transactionHash = outputReferences.back().first;
        tid.input = details;
      }
      inputs.push_back(tid);
    }
  }
}
