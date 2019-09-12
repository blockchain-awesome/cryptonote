// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "crypto.h"
#include "ITransaction.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"

#include "cryptonote/core/account.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/CryptoNoteTools.h"

#include "transfers/TransfersContainer.h"

namespace {

  using namespace cryptonote;
  using namespace crypto;

  inline account_keys_t accountKeysFromKeypairs(
    const key_pair_t& viewKeys, 
    const key_pair_t& spendKeys) {
    account_keys_t ak;
    ak.address.spendPublicKey = spendKeys.publicKey;
    ak.address.viewPublicKey = viewKeys.publicKey;
    ak.spendSecretKey = spendKeys.secretKey;
    ak.viewSecretKey = viewKeys.secretKey;
    return ak;
  }

  inline account_keys_t generateAccountKeys() {
    key_pair_t p1;
    key_pair_t p2;
    generate_keys((uint8_t*)&p2.publicKey, (uint8_t*)&p2.secretKey);
    generate_keys((uint8_t*)&p1.publicKey, (uint8_t*)&p1.secretKey);
    return accountKeysFromKeypairs(p1, p2);
  }

  Account generateAccount() {
    Account account;
    account.generate();
    return account;
  }

  account_public_address_t generateAddress() {
    return generateAccount().getAccountKeys().address;
  }
  
  key_image_t generateKeyImage() {
    return crypto::rand<key_image_t>();
  }

  key_image_t generateKeyImage(const account_keys_t& keys, size_t idx, const public_key_t& txPubKey) {
    key_image_t keyImage;
    cryptonote::key_pair_t in_ephemeral;
    cryptonote::generate_key_image_helper(
     keys,
      txPubKey,
      idx,
      in_ephemeral,
      keyImage);
    return keyImage;
  }

  void addTestInput(ITransaction& transaction, uint64_t amount) {
    key_input_t input;
    input.amount = amount;
    input.keyImage = generateKeyImage();
    input.outputIndexes.emplace_back(1);

    transaction.addInput(input);
  }

  TransactionOutputInformationIn addTestKeyOutput(ITransaction& transaction, uint64_t amount,
    uint32_t globalOutputIndex, const account_keys_t& senderKeys = generateAccountKeys()) {

    uint32_t index = static_cast<uint32_t>(transaction.addOutput(amount, senderKeys.address));

    uint64_t amount_;
    key_output_t output;
    transaction.getOutput(index, output, amount_);

    TransactionOutputInformationIn outputInfo;
    outputInfo.type = TransactionTypes::output_type_t::Key;
    outputInfo.amount = amount_;
    outputInfo.globalOutputIndex = globalOutputIndex;
    outputInfo.outputInTransaction = index;
    outputInfo.transactionPublicKey = transaction.getTransactionPublicKey();
    outputInfo.outputKey = output.key;
    outputInfo.keyImage = generateKeyImage(senderKeys, index, transaction.getTransactionPublicKey());

    return outputInfo;
  }

  inline transaction_t convertTx(ITransactionReader& tx) {
    transaction_t oldTx;
    BinaryArray::from(oldTx, tx.getTransactionData()); // ignore return code
    return oldTx;
  }
}

namespace cryptonote {

class TestTransactionBuilder {
public:

  TestTransactionBuilder();
  TestTransactionBuilder(const binary_array_t& txTemplate, const secret_key_t& secretKey);

  public_key_t getTransactionPublicKey() const;
  void appendExtra(const binary_array_t& extraData);
  void setUnlockTime(uint64_t time);

  // inputs
  size_t addTestInput(uint64_t amount, const account_keys_t& senderKeys = generateAccountKeys());
  size_t addTestInput(uint64_t amount, std::vector<uint32_t> gouts, const account_keys_t& senderKeys = generateAccountKeys());
  void addTestMultisignatureInput(uint64_t amount, const TransactionOutputInformation& t);
  size_t addFakeMultisignatureInput(uint64_t amount, uint32_t globalOutputIndex, size_t signatureCount);
  void addInput(const account_keys_t& senderKeys, const TransactionOutputInformation& t);

  // outputs
  TransactionOutputInformationIn addTestKeyOutput(uint64_t amount, uint32_t globalOutputIndex, const account_keys_t& senderKeys = generateAccountKeys());
  TransactionOutputInformationIn addTestMultisignatureOutput(uint64_t amount, uint32_t globalOutputIndex);
  TransactionOutputInformationIn addTestMultisignatureOutput(uint64_t amount, std::vector<account_public_address_t>& addresses, uint32_t globalOutputIndex);
  size_t addOutput(uint64_t amount, const account_public_address_t& to);
  size_t addOutput(uint64_t amount, const key_output_t& out);
  size_t addOutput(uint64_t amount, const multi_signature_output_t& out);

  // final step
  std::unique_ptr<ITransactionReader> build();

  // get built transaction hash (call only after build)
  hash_t getTransactionHash() const;

private:

  void derivePublicKey(const account_keys_t& reciever, const public_key_t& srcTxKey, size_t outputIndex, public_key_t& ephemeralKey) {
    key_derivation_t derivation;
    generate_key_derivation((const uint8_t*)&srcTxKey, (uint8_t*)&(reciever.viewSecretKey), (uint8_t*)&derivation);
    derive_public_key((const uint8_t*)&derivation, outputIndex,
      (const uint8_t*)&(reciever.address.spendPublicKey),
      (uint8_t*)&(ephemeralKey));
  }

  struct MsigInfo {
    public_key_t transactionKey;
    size_t outputIndex;
    std::vector<Account> accounts;
  };

  std::unordered_map<size_t, std::pair<TransactionTypes::input_key_info_t, key_pair_t>> keys;
  std::unordered_map<size_t, MsigInfo> msigInputs;

  std::unique_ptr<ITransaction> tx;
  hash_t transactionHash;
};

class FusionTransactionBuilder {
public:
  FusionTransactionBuilder(const Currency& currency, uint64_t amount);

  uint64_t getAmount() const;
  void setAmount(uint64_t val);

  uint64_t getFirstInput() const;
  void setFirstInput(uint64_t val);

  uint64_t getFirstOutput() const;
  void setFirstOutput(uint64_t val);

  uint64_t getFee() const;
  void setFee(uint64_t val);

  size_t getExtraSize() const;
  void setExtraSize(size_t val);

  size_t getInputCount() const;
  void setInputCount(size_t val);

  std::unique_ptr<ITransactionReader> buildReader() const;
  transaction_t buildTx() const;

  transaction_t createFusionTransactionBySize(size_t targetSize);

private:
  const Currency& m_currency;
  uint64_t m_amount;
  uint64_t m_firstInput;
  uint64_t m_firstOutput;
  uint64_t m_fee;
  size_t m_extraSize;
  size_t m_inputCount;
};

}

namespace cryptonote {
inline bool operator == (const account_keys_t& a, const account_keys_t& b) { 
  return memcmp(&a, &b, sizeof(a)) == 0; 
}
}
