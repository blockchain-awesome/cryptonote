// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "basics.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>

#include "serialization/ISerializer.h"
#include "serialization/SerializationOverloads.h"
#include "serialization/BinaryInputStreamSerializer.h"
#include "serialization/BinaryOutputStreamSerializer.h"

#include "stream/StringOutputStream.h"
#include "crypto/crypto.h"

#include "cryptonote/core/account.h"

#include "CryptoNoteConfig.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/TransactionExtra.h"

using namespace Common;

namespace {

using namespace cryptonote;
using namespace Common;

size_t getSignaturesCount(const transaction_input_t& input) {
  struct txin_signature_size_visitor : public boost::static_visitor < size_t > {
    size_t operator()(const base_input_t& txin) const { return 0; }
    size_t operator()(const key_input_t& txin) const { return txin.outputIndexes.size(); }
    size_t operator()(const multi_signature_input_t& txin) const { return txin.signatureCount; }
  };

  return boost::apply_visitor(txin_signature_size_visitor(), input);
}

struct BinaryVariantTagGetter: boost::static_visitor<uint8_t> {
  uint8_t operator()(const cryptonote::base_input_t) { return  0xff; }
  uint8_t operator()(const cryptonote::key_input_t) { return  0x2; }
  uint8_t operator()(const cryptonote::multi_signature_input_t) { return  0x3; }
  uint8_t operator()(const cryptonote::key_output_t) { return  0x2; }
  uint8_t operator()(const cryptonote::multi_signature_output_t) { return  0x3; }
  uint8_t operator()(const cryptonote::transaction_t) { return  0xcc; }
  uint8_t operator()(const cryptonote::block_t) { return  0xbb; }
};

struct VariantSerializer : boost::static_visitor<> {
  VariantSerializer(cryptonote::ISerializer& serializer, const std::string& name) : s(serializer), name(name) {}

  template <typename T>
  void operator() (T& param) { s(param, name); }

  cryptonote::ISerializer& s;
  std::string name;
};

void getVariantValue(cryptonote::ISerializer& serializer, uint8_t tag, cryptonote::transaction_input_t& in) {
  switch(tag) {
  case 0xff: {
    cryptonote::base_input_t v;
    serializer(v, "value");
    in = v;
    break;
  }
  case 0x2: {
    cryptonote::key_input_t v;
    serializer(v, "value");
    in = v;
    break;
  }
  case 0x3: {
    cryptonote::multi_signature_input_t v;
    serializer(v, "value");
    in = v;
    break;
  }
  default:
    throw std::runtime_error("Unknown variant tag");
  }
}

void getVariantValue(cryptonote::ISerializer& serializer, uint8_t tag, cryptonote::transaction_output_target_t& out) {
  switch(tag) {
  case 0x2: {
    cryptonote::key_output_t v;
    serializer(v, "data");
    out = v;
    break;
  }
  case 0x3: {
    cryptonote::multi_signature_output_t v;
    serializer(v, "data");
    out = v;
    break;
  }
  default:
    throw std::runtime_error("Unknown variant tag");
  }
}

bool serializeVarintVector(std::vector<uint32_t>& vector, cryptonote::ISerializer& serializer, Common::StringView name) {
  size_t size = vector.size();
  
  if (!serializer.beginArray(size, name)) {
    vector.clear();
    return false;
  }

  vector.resize(size);

  for (size_t i = 0; i < size; ++i) {
    serializer(vector[i], "");
  }

  serializer.endArray();
  return true;
}

}

namespace cryptonote {

void serialize(transaction_prefix_t& txP, ISerializer& serializer) {
  serializer(txP.version, "version");

  if (CURRENT_TRANSACTION_VERSION < txP.version) {
    throw std::runtime_error("Wrong transaction version");
  }

  serializer(txP.unlockTime, "unlock_time");
  serializer(txP.inputs, "vin");
  serializer(txP.outputs, "vout");
  serializeAsBinary(txP.extra, "extra", serializer);
}

void serialize(transaction_t& tx, ISerializer& serializer) {
  serialize(static_cast<transaction_prefix_t&>(tx), serializer);

  size_t sigSize = tx.inputs.size();
  //TODO: make arrays without sizes
//  serializer.beginArray(sigSize, "signatures");
  
  if (serializer.type() == ISerializer::INPUT) {
    tx.signatures.resize(sigSize);
  }

  bool signaturesNotExpected = tx.signatures.empty();
  if (!signaturesNotExpected && tx.inputs.size() != tx.signatures.size()) {
    throw std::runtime_error("Serialization error: unexpected signatures size");
  }

  for (size_t i = 0; i < tx.inputs.size(); ++i) {
    size_t signatureSize = getSignaturesCount(tx.inputs[i]);
    if (signaturesNotExpected) {
      if (signatureSize == 0) {
        continue;
      } else {
        throw std::runtime_error("Serialization error: signatures are not expected");
      }
    }

    if (serializer.type() == ISerializer::OUTPUT) {
      if (signatureSize != tx.signatures[i].size()) {
        throw std::runtime_error("Serialization error: unexpected signatures size");
      }

      for (crypto::signature_t& sig : tx.signatures[i]) {
        serializePod(sig, "", serializer);
      }

    } else {
      std::vector<crypto::signature_t> signatures(signatureSize);
      for (crypto::signature_t& sig : signatures) {
        serializePod(sig, "", serializer);
      }

      tx.signatures[i] = std::move(signatures);
    }
  }
//  serializer.endArray();
}

void serialize(transaction_input_t& in, ISerializer& serializer) {
  if (serializer.type() == ISerializer::OUTPUT) {
    BinaryVariantTagGetter tagGetter;
    uint8_t tag = boost::apply_visitor(tagGetter, in);
    serializer.binary(&tag, sizeof(tag), "type");

    VariantSerializer visitor(serializer, "value");
    boost::apply_visitor(visitor, in);
  } else {
    uint8_t tag;
    serializer.binary(&tag, sizeof(tag), "type");

    getVariantValue(serializer, tag, in);
  }
}

void serialize(base_input_t& gen, ISerializer& serializer) {
  serializer(gen.blockIndex, "height");
}

void serialize(key_input_t& key, ISerializer& serializer) {
  serializer(key.amount, "amount");
  serializeVarintVector(key.outputIndexes, serializer, "key_offsets");
  serializer(key.keyImage, "k_image");
}

void serialize(multi_signature_input_t& multisignature, ISerializer& serializer) {
  serializer(multisignature.amount, "amount");
  serializer(multisignature.signatureCount, "signatures");
  serializer(multisignature.outputIndex, "outputIndex");
}

void serialize(transaction_output_t& output, ISerializer& serializer) {
  serializer(output.amount, "amount");
  serializer(output.target, "target");
}

void serialize(transaction_output_target_t& output, ISerializer& serializer) {
  if (serializer.type() == ISerializer::OUTPUT) {
    BinaryVariantTagGetter tagGetter;
    uint8_t tag = boost::apply_visitor(tagGetter, output);
    serializer.binary(&tag, sizeof(tag), "type");

    VariantSerializer visitor(serializer, "data");
    boost::apply_visitor(visitor, output);
  } else {
    uint8_t tag;
    serializer.binary(&tag, sizeof(tag), "type");

    getVariantValue(serializer, tag, output);
  }
}

void serialize(key_output_t& key, ISerializer& serializer) {
  serializer(key.key, "key");
}

void serialize(multi_signature_output_t& multisignature, ISerializer& serializer) {
  serializer(multisignature.keys, "keys");
  serializer(multisignature.requiredSignatureCount, "required_signatures");
}

void serializeBlockHeader(block_header_t& header, ISerializer& serializer) {
  serializer(header.majorVersion, "major_version");
  // if (header.majorVersion > BLOCK_MAJOR_VERSION_1) {
  //   throw std::runtime_error("Wrong major version");
  // }

  serializer(header.minorVersion, "minor_version");
  serializer(header.timestamp, "timestamp");
  serializer(header.previousBlockHash, "prev_id");
  serializer.binary(&header.nonce, sizeof(header.nonce), "nonce");
}

void serialize(block_header_t& header, ISerializer& serializer) {
  serializeBlockHeader(header, serializer);
}

void serialize(block_t& block, ISerializer& serializer) {
  serializeBlockHeader(block, serializer);

  serializer(block.baseTransaction, "miner_tx");
  serializer(block.transactionHashes, "tx_hashes");
}

void serialize(account_public_address_t& address, ISerializer& serializer) {
  serializer(address.spendPublicKey, "m_spend_public_key");
  serializer(address.viewPublicKey, "m_view_public_key");
}

void serialize(account_keys_t& keys, ISerializer& s) {
  s(keys.address, "m_account_address");
  s(keys.spendSecretKey, "m_spend_secret_key");
  s(keys.viewSecretKey, "m_view_secret_key");
}

void serialize(key_pair_t& keyPair, ISerializer& serializer) {
  serializer(keyPair.secretKey, "secret_key");
  serializer(keyPair.publicKey, "public_key");
}


} //namespace cryptonote
