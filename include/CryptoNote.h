// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include <boost/variant.hpp>
#include "CryptoTypes.h"

namespace cryptonote {

struct BaseInput {
  uint32_t blockIndex;
};

struct KeyInput {
  uint64_t amount;
  std::vector<uint32_t> outputIndexes;
  crypto::KeyImage keyImage;
};

struct MultisignatureInput {
  uint64_t amount;
  uint8_t signatureCount;
  uint32_t outputIndex;
};

struct KeyOutput {
  crypto::PublicKey key;
};

struct MultisignatureOutput {
  std::vector<crypto::PublicKey> keys;
  uint8_t requiredSignatureCount;
};

typedef boost::variant<BaseInput, KeyInput, MultisignatureInput> TransactionInput;

typedef boost::variant<KeyOutput, MultisignatureOutput> TransactionOutputTarget;

struct TransactionOutput {
  uint64_t amount;
  TransactionOutputTarget target;
};

struct TransactionPrefix {
  uint8_t version;
  uint64_t unlockTime;
  std::vector<TransactionInput> inputs;
  std::vector<TransactionOutput> outputs;
  std::vector<uint8_t> extra;
};

struct Transaction : public TransactionPrefix {
  std::vector<std::vector<crypto::Signature>> signatures;
};

struct BlockHeader {
  uint8_t majorVersion;
  uint8_t minorVersion;
  uint32_t nonce;
  uint64_t timestamp;
  crypto::Hash previousBlockHash;
};

struct Block : public BlockHeader {
  Transaction baseTransaction;
  std::vector<crypto::Hash> transactionHashes;
};

struct AccountPublicAddress {
  crypto::PublicKey spendPublicKey;
  crypto::PublicKey viewPublicKey;
};

struct AccountKeys {
  AccountPublicAddress address;
  crypto::SecretKey spendSecretKey;
  crypto::SecretKey viewSecretKey;
};

struct KeyPair {
  crypto::PublicKey publicKey;
  crypto::SecretKey secretKey;
};

using BinaryArray = std::vector<uint8_t>;

}
