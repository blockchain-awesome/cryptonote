// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CryptoNoteTools.h"
#include "CryptoNoteFormatUtils.h"

namespace cryptonote {
template<>
bool toBinaryArray(const BinaryArray& object, BinaryArray& binaryArray) {
  try {
    Common::VectorOutputStream stream(binaryArray);
    BinaryOutputStreamSerializer serializer(stream);
    std::string oldBlob = Common::asString(object);
    serializer(oldBlob, "");
  } catch (std::exception&) {
    return false;
  }

  return true;
}

void getBinaryArrayHash(const BinaryArray& binaryArray, crypto::hash_t& hash) {
  cn_fast_hash(binaryArray.data(), binaryArray.size(), hash);
}

crypto::hash_t getBinaryArrayHash(const BinaryArray& binaryArray) {
  crypto::hash_t hash;
  getBinaryArrayHash(binaryArray, hash);
  return hash;
}

uint64_t getInputAmount(const transaction_t& transaction) {
  uint64_t amount = 0;
  for (auto& input : transaction.inputs) {
    if (input.type() == typeid(key_input_t)) {
      amount += boost::get<key_input_t>(input).amount;
    } else if (input.type() == typeid(multi_signature_input_t)) {
      amount += boost::get<multi_signature_input_t>(input).amount;
    }
  }

  return amount;
}

std::vector<uint64_t> getInputsAmounts(const transaction_t& transaction) {
  std::vector<uint64_t> inputsAmounts;
  inputsAmounts.reserve(transaction.inputs.size());

  for (auto& input: transaction.inputs) {
    if (input.type() == typeid(key_input_t)) {
      inputsAmounts.push_back(boost::get<key_input_t>(input).amount);
    } else if (input.type() == typeid(multi_signature_input_t)) {
      inputsAmounts.push_back(boost::get<multi_signature_input_t>(input).amount);
    }
  }

  return inputsAmounts;
}

uint64_t getOutputAmount(const transaction_t& transaction) {
  uint64_t amount = 0;
  for (auto& output : transaction.outputs) {
    amount += output.amount;
  }

  return amount;
}

void decomposeAmount(uint64_t amount, uint64_t dustThreshold, std::vector<uint64_t>& decomposedAmounts) {
  decompose_amount_into_digits(amount, dustThreshold,
    [&](uint64_t amount) {
    decomposedAmounts.push_back(amount);
  },
    [&](uint64_t dust) {
    decomposedAmounts.push_back(dust);
  }
  );
}

}
