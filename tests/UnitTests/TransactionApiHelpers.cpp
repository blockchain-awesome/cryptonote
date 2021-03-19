// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionApiHelpers.h"
#include "cryptonote/core/transaction/TransactionApi.h"

using namespace cryptonote;
using namespace crypto;

namespace {

  const std::vector<Account>& getMsigAccounts() {
    static std::vector<Account> msigAccounts = { generateAccount(), generateAccount() };
    return msigAccounts;
  }

}

TestTransactionBuilder::TestTransactionBuilder() {
  tx = createTransaction();
}

TestTransactionBuilder::TestTransactionBuilder(const binary_array_t& txTemplate, const secret_key_t& secretKey) {
  tx = createTransaction(txTemplate);
  tx->setTransactionSecretKey(secretKey);
}

public_key_t TestTransactionBuilder::getTransactionPublicKey() const {
  return tx->getTransactionPublicKey();
}

void TestTransactionBuilder::appendExtra(const binary_array_t& extraData) {
  tx->appendExtra(extraData);
}

void TestTransactionBuilder::setUnlockTime(uint64_t time) {
  tx->setUnlockTime(time);
}

size_t TestTransactionBuilder::addTestInput(uint64_t amount, const account_keys_t& senderKeys) {
  using namespace TransactionTypes;

  TransactionTypes::input_key_info_t info;
  public_key_t targetKey;

  // cryptonote::key_pair_t srcTxKeys = cryptonote::generateKeyPair();
  key_pair_t srcTxKeys = Key::generate();
  derivePublicKey(senderKeys, srcTxKeys.publicKey, 5, targetKey);

  TransactionTypes::global_output_t gout = { targetKey, 0 };

  info.amount = amount;
  info.outputs.push_back(gout);

  info.realOutput.transactionIndex = 0;
  info.realOutput.outputInTransaction = 5;
  info.realOutput.transactionPublicKey = reinterpret_cast<const public_key_t&>(srcTxKeys.publicKey);

  key_pair_t ephKeys;
  size_t idx = tx->addInput(senderKeys, info, ephKeys);
  keys[idx] = std::make_pair(info, ephKeys);
  return idx;
}

size_t TestTransactionBuilder::addTestInput(uint64_t amount, std::vector<uint32_t> gouts, const account_keys_t& senderKeys) {
  using namespace TransactionTypes;

  TransactionTypes::input_key_info_t info;
  public_key_t targetKey;

  // cryptonote::key_pair_t srcTxKeys = cryptonote::generateKeyPair();
  key_pair_t srcTxKeys = Key::generate();

  derivePublicKey(senderKeys, srcTxKeys.publicKey, 5, targetKey);

  TransactionTypes::global_output_t gout = { targetKey, 0 };

  info.amount = amount;
  info.outputs.push_back(gout);
  public_key_t pk;
  secret_key_t sk;
  for (auto out : gouts) {
    generate_keys((uint8_t*)&pk, (uint8_t*)&sk);
    info.outputs.push_back(TransactionTypes::global_output_t{ pk, out });
  }

  info.realOutput.transactionIndex = 0;
  info.realOutput.outputInTransaction = 5;
  info.realOutput.transactionPublicKey = reinterpret_cast<const public_key_t&>(srcTxKeys.publicKey);

  key_pair_t ephKeys;
  size_t idx = tx->addInput(senderKeys, info, ephKeys);
  keys[idx] = std::make_pair(info, ephKeys);
  return idx;
}

void TestTransactionBuilder::addInput(const account_keys_t& senderKeys, const TransactionOutputInformation& t) {
  TransactionTypes::input_key_info_t info;
  info.amount = t.amount;

  TransactionTypes::global_output_t globalOut;
  globalOut.outputIndex = t.globalOutputIndex;
  globalOut.targetKey = t.outputKey;
  info.outputs.push_back(globalOut);

  info.realOutput.outputInTransaction = t.outputInTransaction;
  info.realOutput.transactionIndex = 0;
  info.realOutput.transactionPublicKey = t.transactionPublicKey;

  key_pair_t ephKeys;
  size_t idx = tx->addInput(senderKeys, info, ephKeys);
  keys[idx] = std::make_pair(info, ephKeys);
}

void TestTransactionBuilder::addTestMultisignatureInput(uint64_t amount, const TransactionOutputInformation& t) {
  multi_signature_input_t input;
  input.amount = amount;
  input.outputIndex = t.globalOutputIndex;
  input.signatureCount = t.requiredSignatures;
  size_t idx = tx->addInput(input);
 
  msigInputs[idx] = MsigInfo{ t.transactionPublicKey, t.outputInTransaction, getMsigAccounts() };
}

size_t TestTransactionBuilder::addFakeMultisignatureInput(uint64_t amount, uint32_t globalOutputIndex, size_t signatureCount) {
  multi_signature_input_t input;
  input.amount = amount;
  input.outputIndex = globalOutputIndex;
  input.signatureCount = static_cast<uint8_t>(signatureCount);
  size_t idx = tx->addInput(input);

  std::vector<Account> accs;
  for (size_t i = 0; i < signatureCount; ++i) {
    accs.push_back(generateAccount());
  }

  msigInputs[idx] = MsigInfo{ crypto::rand<public_key_t>(), 0, std::move(accs) };
  return idx;
}

TransactionOutputInformationIn TestTransactionBuilder::addTestKeyOutput(uint64_t amount, uint32_t globalOutputIndex, const account_keys_t& senderKeys) {
  uint32_t index = static_cast<uint32_t>(tx->addOutput(amount, senderKeys.address));

  uint64_t amount_;
  key_output_t output;
  tx->getOutput(index, output, amount_);

  TransactionOutputInformationIn outputInfo;
  outputInfo.type = TransactionTypes::output_type_t::Key;
  outputInfo.amount = amount_;
  outputInfo.globalOutputIndex = globalOutputIndex;
  outputInfo.outputInTransaction = index;
  outputInfo.transactionPublicKey = tx->getTransactionPublicKey();
  outputInfo.outputKey = output.key;
  outputInfo.keyImage = generateKeyImage(senderKeys, index, tx->getTransactionPublicKey());

  return outputInfo;
}

TransactionOutputInformationIn TestTransactionBuilder::addTestMultisignatureOutput(uint64_t amount, std::vector<account_public_address_t>& addresses, uint32_t globalOutputIndex) {
  uint32_t index = static_cast<uint32_t>(tx->addOutput(amount, addresses, static_cast<uint32_t>(addresses.size())));

  uint64_t _amount;
  multi_signature_output_t output;
  tx->getOutput(index, output, _amount);

  TransactionOutputInformationIn outputInfo;
  outputInfo.type = TransactionTypes::output_type_t::Multisignature;
  outputInfo.amount = _amount;
  outputInfo.globalOutputIndex = globalOutputIndex;
  outputInfo.outputInTransaction = index;
  outputInfo.transactionPublicKey = tx->getTransactionPublicKey();
  // Doesn't used in multisignature output, so can contain garbage
  outputInfo.keyImage = generateKeyImage();
  outputInfo.requiredSignatures = output.requiredSignatureCount;
  return outputInfo;
}

TransactionOutputInformationIn TestTransactionBuilder::addTestMultisignatureOutput(uint64_t amount, uint32_t globalOutputIndex) {
  std::vector<account_public_address_t> multisigAddresses;
  for (const auto& acc : getMsigAccounts()) {
    multisigAddresses.push_back(acc.getAccountKeys().address);
  }

  return addTestMultisignatureOutput(amount, multisigAddresses, globalOutputIndex);
}

size_t TestTransactionBuilder::addOutput(uint64_t amount, const account_public_address_t& to) {
  return tx->addOutput(amount, to);
}

size_t TestTransactionBuilder::addOutput(uint64_t amount, const key_output_t& out) {
  return tx->addOutput(amount, out);
}

size_t TestTransactionBuilder::addOutput(uint64_t amount, const multi_signature_output_t& out) {
  return tx->addOutput(amount, out);
}

std::unique_ptr<ITransactionReader> TestTransactionBuilder::build() {
  for (const auto& kv : keys) {
    tx->signInputKey(kv.first, kv.second.first, kv.second.second);
  }

  for (const auto& kv : msigInputs) {
    for (const auto& acc : kv.second.accounts) {
      tx->signInputMultisignature(kv.first, kv.second.transactionKey, kv.second.outputIndex, acc.getAccountKeys());
    }
  }

  transactionHash = tx->getTransactionHash();

  keys.clear();
  return std::move(tx);
}

hash_t TestTransactionBuilder::getTransactionHash() const {
  return transactionHash;
}

FusionTransactionBuilder::FusionTransactionBuilder(const Currency& currency, uint64_t amount) :
  m_currency(currency),
  m_amount(amount),
  m_firstInput(0),
  m_firstOutput(0),
  m_fee(0),
  m_extraSize(0),
  m_inputCount(currency.fusionTxMinInputCount()) {
}

uint64_t FusionTransactionBuilder::getAmount() const {
  return m_amount;
}

void FusionTransactionBuilder::setAmount(uint64_t val) {
  m_amount = val;
}

uint64_t FusionTransactionBuilder::getFirstInput() const {
  return m_firstInput;
}

void FusionTransactionBuilder::setFirstInput(uint64_t val) {
  m_firstInput = val;
}

uint64_t FusionTransactionBuilder::getFirstOutput() const {
  return m_firstOutput;
}

void FusionTransactionBuilder::setFirstOutput(uint64_t val) {
  m_firstOutput = val;
}

uint64_t FusionTransactionBuilder::getFee() const {
  return m_fee;
}

void FusionTransactionBuilder::setFee(uint64_t val) {
  m_fee = val;
}

size_t FusionTransactionBuilder::getExtraSize() const {
  return m_extraSize;
}

void FusionTransactionBuilder::setExtraSize(size_t val) {
  m_extraSize = val;
}

size_t FusionTransactionBuilder::getInputCount() const {
  return m_inputCount;
}

void FusionTransactionBuilder::setInputCount(size_t val) {
  m_inputCount = val;
}

std::unique_ptr<ITransactionReader> FusionTransactionBuilder::buildReader() const {
  assert(m_inputCount > 0);
  assert(m_firstInput <= m_amount);
  assert(m_amount > m_currency.defaultDustThreshold());

  TestTransactionBuilder builder;

  if (m_extraSize != 0) {
    builder.appendExtra(binary_array_t(m_extraSize, 0));
  }

  if (m_firstInput != 0) {
    builder.addTestInput(m_firstInput);
  }

  if (m_amount > m_firstInput) {
    builder.addTestInput(m_amount - m_firstInput - (m_inputCount - 1) * m_currency.defaultDustThreshold());
    for (size_t i = 0; i < m_inputCount - 1; ++i) {
      builder.addTestInput(m_currency.defaultDustThreshold());
    }
  }

  account_public_address_t address = generateAddress();
  std::vector<uint64_t> outputAmounts;
  assert(m_amount >= m_firstOutput + m_fee);
  decomposeAmount(m_amount - m_firstOutput - m_fee, m_currency.defaultDustThreshold(), outputAmounts);
  std::sort(outputAmounts.begin(), outputAmounts.end());

  if (m_firstOutput != 0) {
    builder.addOutput(m_firstOutput, address);
  }

  for (auto outAmount : outputAmounts) {
    builder.addOutput(outAmount, address);
  }

  return builder.build();
}

transaction_t FusionTransactionBuilder::buildTx() const {
  return convertTx(*buildReader());
}

transaction_t FusionTransactionBuilder::createFusionTransactionBySize(size_t targetSize) {
  auto tx = buildReader();

  size_t realSize = tx->getTransactionData().size();
  if (realSize < targetSize) {
    setExtraSize(targetSize - realSize);
    tx = buildReader();

    realSize = tx->getTransactionData().size();
    if (realSize > targetSize) {
      setExtraSize(getExtraSize() - 1);
      tx = buildReader();
    }
  }

  return convertTx(*tx);
}
