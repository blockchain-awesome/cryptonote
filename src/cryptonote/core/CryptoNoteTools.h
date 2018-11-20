// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <limits>
#include "stream/MemoryInputStream.h"
#include "common/StringTools.h"
#include "stream/VectorOutputStream.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include "serialization/BinaryInputStreamSerializer.h"
#include "cryptonote/core/blockchain/serializer/basics.h"

namespace cryptonote {
template<class T>
bool getObjectHash(const T& object, crypto::hash_t& hash) {
  binary_array_t ba;
  if (!BinaryArray::to(object, ba)) {
    hash = NULL_HASH;
    return false;
  }

  hash = BinaryArray::getHash(ba);
  return true;
}

template<class T>
bool getObjectHash(const T& object, crypto::hash_t& hash, size_t& size) {
  binary_array_t ba;
  if (!BinaryArray::to(object, ba)) {
    hash = NULL_HASH;
    size = (std::numeric_limits<size_t>::max)();
    return false;
  }

  size = ba.size();
  hash = BinaryArray::getHash(ba);
  return true;
}

template<class T>
crypto::hash_t getObjectHash(const T& object) {
  crypto::hash_t hash;
  getObjectHash(object, hash);
  return hash;
}

uint64_t getInputAmount(const transaction_t& transaction);
std::vector<uint64_t> getInputsAmounts(const transaction_t& transaction);
uint64_t getOutputAmount(const transaction_t& transaction);
void decomposeAmount(uint64_t amount, uint64_t dustThreshold, std::vector<uint64_t>& decomposedAmounts);
}
