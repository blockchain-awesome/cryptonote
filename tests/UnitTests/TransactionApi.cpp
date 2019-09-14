// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <numeric>
#include <random>

#include "cryptonote/core/TransactionApi.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h" // TODO: delete
#include "cryptonote/core/account.h"
#include "cryptonote/crypto/crypto.h"
#include "TransactionApiHelpers.h"

using namespace cryptonote;

namespace {
 
  template <size_t size> 
  void fillRandomBytes(std::array<uint8_t, size>& data) {
    for (size_t i = 0; i < size; ++i) {
      data[i] = std::rand() % std::numeric_limits<uint8_t>::max();
    }
  }
  
  template <typename Array>
  Array randomArray() {
    Array a;
    fillRandomBytes(a);
    return a;
  }

  void derivePublicKey(const account_keys_t& reciever, const public_key_t& srcTxKey, size_t outputIndex, public_key_t& ephemeralKey) {
    key_derivation_t derivation;
    generate_key_derivation((const uint8_t*)&srcTxKey, (const uint8_t*)&(reciever.viewSecretKey), (uint8_t*)&derivation);
    derive_public_key((const uint8_t*)&derivation, outputIndex, 
      (const uint8_t*)&(reciever.address.spendPublicKey), 
      (uint8_t*)&(ephemeralKey));
  }


  std::unique_ptr<ITransaction> reloadedTx(const std::unique_ptr<ITransaction>& tx) {
    auto txBlob = tx->getTransactionData();
    return createTransaction(txBlob);
  }

  void checkTxReload(const std::unique_ptr<ITransaction>& tx) {
    auto txBlob = tx->getTransactionData();
    auto tx2 = createTransaction(txBlob);
    ASSERT_EQ(tx2->getTransactionData(), txBlob);
  }


  class TransactionApi : public testing::Test {
  protected:
    
    virtual void SetUp() override {
      sender = generateAccountKeys();
      tx = createTransaction();
      txHash = tx->getTransactionHash();
    }

    TransactionTypes::input_key_info_t createInputInfo(uint64_t amount) {
      TransactionTypes::input_key_info_t info;

      // cryptonote::key_pair_t srcTxKeys = cryptonote::generateKeyPair();
      key_pair_t srcTxKeys = Key::generate();

      public_key_t targetKey;

      derivePublicKey(sender, srcTxKeys.publicKey, 5, targetKey);

      TransactionTypes::global_output_t gout = { targetKey, 0 };

      info.amount = 1000;
      info.outputs.push_back(gout);

      info.realOutput.transactionIndex = 0;
      info.realOutput.outputInTransaction = 5;
      info.realOutput.transactionPublicKey = reinterpret_cast<const public_key_t&>(srcTxKeys.publicKey);

      return info;
    }

    void checkHashChanged() {
      auto txNewHash = tx->getTransactionHash();
      EXPECT_NE(txHash, txNewHash);
      txHash = txNewHash;
    }

    void checkHashUnchanged() {
      EXPECT_EQ(txHash, tx->getTransactionHash());
    }


    account_keys_t sender;
    std::unique_ptr<ITransaction> tx;
    hash_t txHash;
  };

}

TEST_F(TransactionApi, createEmptyReload) {
  auto hash = tx->getTransactionHash();
  auto pk = tx->getTransactionPublicKey();
  checkTxReload(tx);
  // transaction key should not change on reload
  auto reloaded = reloadedTx(tx);
  ASSERT_EQ(pk, reloaded->getTransactionPublicKey());
  ASSERT_EQ(hash, reloaded->getTransactionHash());
}

TEST_F(TransactionApi, addAndSignInput) {
  ASSERT_EQ(0, tx->getInputCount());
  ASSERT_EQ(0, tx->getInputTotalAmount());

  TransactionTypes::input_key_info_t info = createInputInfo(1000);
  key_pair_t ephKeys;
  size_t index = tx->addInput(sender, info, ephKeys);

  ASSERT_EQ(0, index);
  ASSERT_EQ(1, tx->getInputCount());
  ASSERT_EQ(1000, tx->getInputTotalAmount());
  ASSERT_EQ(TransactionTypes::input_type_t::Key, tx->getInputType(index));
  ASSERT_EQ(1, tx->getRequiredSignaturesCount(index));

  ASSERT_TRUE(tx->validateInputs());
  ASSERT_FALSE(tx->validateSignatures()); // signature not present

  tx->signInputKey(index, info, ephKeys);

  ASSERT_TRUE(tx->validateSignatures()); // now it's ok

  auto txBlob = tx->getTransactionData();
  ASSERT_FALSE(txBlob.empty());

  EXPECT_NO_FATAL_FAILURE(checkHashChanged());
}

TEST_F(TransactionApi, addAndSignInputMsig) {

  multi_signature_input_t inputMsig;

  inputMsig.amount = 1000;
  inputMsig.outputIndex = 0;
  inputMsig.signatureCount = 3;

  auto index = tx->addInput(inputMsig);

  ASSERT_EQ(0, index);
  ASSERT_EQ(1, tx->getInputCount());
  ASSERT_EQ(1000, tx->getInputTotalAmount());
  ASSERT_EQ(TransactionTypes::input_type_t::Multisignature, tx->getInputType(index));
  ASSERT_EQ(3, tx->getRequiredSignaturesCount(index));

  key_pair_t kp1;
  generate_keys((uint8_t*)&kp1.publicKey, (uint8_t*)&kp1.secretKey );

  auto srcTxKey = kp1.publicKey;
  account_keys_t accounts[] = { generateAccountKeys(), generateAccountKeys(), generateAccountKeys() };

  tx->signInputMultisignature(index, srcTxKey, 0, accounts[0]);

  ASSERT_FALSE(tx->validateSignatures());

  tx->signInputMultisignature(index, srcTxKey, 0, accounts[1]);
  tx->signInputMultisignature(index, srcTxKey, 0, accounts[2]);

  ASSERT_TRUE(tx->validateSignatures());

  auto txBlob = tx->getTransactionData();
  ASSERT_FALSE(txBlob.empty());
  EXPECT_NO_FATAL_FAILURE(checkHashChanged());
}

TEST_F(TransactionApi, addOutputKey) {
  ASSERT_EQ(0, tx->getOutputCount());
  ASSERT_EQ(0, tx->getOutputTotalAmount());

  size_t index = tx->addOutput(1000, sender.address);

  ASSERT_EQ(0, index);
  ASSERT_EQ(1, tx->getOutputCount());
  ASSERT_EQ(1000, tx->getOutputTotalAmount());
  ASSERT_EQ(TransactionTypes::output_type_t::Key, tx->getOutputType(index));
  EXPECT_NO_FATAL_FAILURE(checkHashChanged());
}

TEST_F(TransactionApi, addOutputMsig) {
  ASSERT_EQ(0, tx->getOutputCount());
  ASSERT_EQ(0, tx->getOutputTotalAmount());

  account_keys_t accounts[] = { generateAccountKeys(), generateAccountKeys(), generateAccountKeys() };
  std::vector<account_public_address_t> targets;

  for (size_t i = 0; i < sizeof(accounts)/sizeof(accounts[0]); ++i)
    targets.push_back(accounts[i].address);

  size_t index = tx->addOutput(1000, targets, 2);

  ASSERT_EQ(0, index);
  ASSERT_EQ(1, tx->getOutputCount());
  ASSERT_EQ(1000, tx->getOutputTotalAmount());
  ASSERT_EQ(TransactionTypes::output_type_t::Multisignature, tx->getOutputType(index));
  EXPECT_NO_FATAL_FAILURE(checkHashChanged());
}

TEST_F(TransactionApi, secretKey) {
  tx->addOutput(1000, sender.address);
  ASSERT_EQ(1000, tx->getOutputTotalAmount()); 
  // reloaded transaction does not have secret key, cannot add outputs
  auto tx2 = reloadedTx(tx);
  ASSERT_ANY_THROW(tx2->addOutput(1000, sender.address));
  // take secret key from first transaction and add to second (reloaded)
  secret_key_t txSecretKey;
  ASSERT_TRUE(tx->getTransactionSecretKey(txSecretKey));
  
  key_pair_t kp1;
  generate_keys((uint8_t*)&kp1.publicKey, (uint8_t*)&kp1.secretKey);
  secret_key_t sk = kp1.secretKey;
  ASSERT_ANY_THROW(tx2->setTransactionSecretKey(sk)); // unrelated secret key should not be accepted
  
  tx2->setTransactionSecretKey(txSecretKey);
  // adding output should succeed
  tx2->addOutput(500, sender.address);
  ASSERT_EQ(1500, tx2->getOutputTotalAmount());
}

TEST_F(TransactionApi, prefixHash) {
  auto hash = tx->getTransactionPrefixHash();
  tx->addOutput(1000, sender.address);
  // transaction hash should change
  ASSERT_NE(hash, tx->getTransactionPrefixHash());
  hash = tx->getTransactionPrefixHash();
  // prefix hash should not change on reload
  ASSERT_EQ(hash, reloadedTx(tx)->getTransactionPrefixHash());
}

TEST_F(TransactionApi, findOutputs) {
  account_keys_t accounts[] = { generateAccountKeys(), generateAccountKeys(), generateAccountKeys() };

  tx->addOutput(1111, accounts[0].address);
  tx->addOutput(2222, accounts[1].address);
  tx->addOutput(3333, accounts[2].address);

  std::vector<uint32_t> outs;
  uint64_t amount = 0;

  tx->findOutputsToAccount(accounts[2].address, accounts[2].viewSecretKey, outs, amount);

  ASSERT_EQ(1, outs.size());
  ASSERT_EQ(2, outs[0]);
  ASSERT_EQ(3333, amount);
}

TEST_F(TransactionApi, setGetPaymentId) {
  hash_t paymentId = crypto::rand<hash_t>();

  ASSERT_FALSE(tx->getPaymentId(paymentId));

  tx->setPaymentId(paymentId);

  EXPECT_NO_FATAL_FAILURE(checkHashChanged());

  hash_t paymentId2;
  ASSERT_TRUE(tx->getPaymentId(paymentId2));
  ASSERT_EQ(paymentId, paymentId2);

  auto tx2 = reloadedTx(tx);

  hash_t paymentId3;
  ASSERT_TRUE(tx->getPaymentId(paymentId3));
  ASSERT_EQ(paymentId, paymentId3);
}

TEST_F(TransactionApi, setExtraNonce) {
  binary_array_t extraNonce = array::fromString("Hello, world"); // just a sequence of bytes
  binary_array_t s;

  ASSERT_FALSE(tx->getExtraNonce(s));
  tx->setExtraNonce(extraNonce);

  ASSERT_TRUE(tx->getExtraNonce(s));
  ASSERT_EQ(extraNonce, s);

  s.clear();

  ASSERT_TRUE(reloadedTx(tx)->getExtraNonce(s));
  ASSERT_EQ(extraNonce, s);
}

TEST_F(TransactionApi, appendExtra) {
  binary_array_t ba;

  ba.resize(100);
  std::iota(ba.begin(), ba.end(), 0);

  auto tx = createTransaction();

  auto extra = tx->getExtra();

  ASSERT_FALSE(extra.empty());

  tx->appendExtra(ba);

  auto newExtra = tx->getExtra();

  ASSERT_EQ(ba.size() + extra.size(), newExtra.size());
  ASSERT_EQ(0, memcmp(newExtra.data() + extra.size(), ba.data(), ba.size()));
}


TEST_F(TransactionApi, doubleSpendInTransactionKey) {
  TransactionTypes::input_key_info_t info = createInputInfo(1000);

  key_pair_t ephKeys;
  tx->addInput(sender, info, ephKeys);
  ASSERT_TRUE(tx->validateInputs());
  // now, add the same output again
  tx->addInput(sender, info, ephKeys);
  ASSERT_FALSE(tx->validateInputs());
}

TEST_F(TransactionApi, doubleSpendInTransactionMultisignature) {
  multi_signature_input_t inputMsig = { 1000, 0, 2 };

  tx->addInput(inputMsig);
  ASSERT_TRUE(tx->validateInputs());
  tx->addInput(inputMsig);
  ASSERT_FALSE(tx->validateInputs());
}


TEST_F(TransactionApi, unableToModifySignedTransaction) {

  multi_signature_input_t inputMsig;

  inputMsig.amount = 1000;
  inputMsig.outputIndex = 0;
  inputMsig.signatureCount = 2;
  auto index = tx->addInput(inputMsig);

  key_pair_t kp1;
  generate_keys((uint8_t*)&kp1.publicKey, (uint8_t*)&kp1.secretKey);

  auto srcTxKey = kp1.publicKey;

  tx->signInputMultisignature(index, srcTxKey, 0, generateAccountKeys());

  // from now on, we cannot modify transaction prefix
  ASSERT_ANY_THROW(tx->addInput(inputMsig));
  ASSERT_ANY_THROW(tx->addOutput(500, sender.address));

  hash_t paymentId;
  ASSERT_ANY_THROW(tx->setPaymentId(paymentId));
  ASSERT_ANY_THROW(tx->setExtraNonce(array::fromString("smth")));

  // but can add more signatures
  tx->signInputMultisignature(index, srcTxKey, 0, generateAccountKeys());

  EXPECT_NO_FATAL_FAILURE(checkHashChanged());
}
