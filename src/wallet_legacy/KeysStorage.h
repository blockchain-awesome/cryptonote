// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "crypto/crypto.h"

#include <stdint.h>

using namespace crypto;

namespace cryptonote {

class ISerializer;

//This is DTO structure. Do not change it.
struct KeysStorage {
  uint64_t creationTimestamp;

  public_key_t spendPublicKey;
  secret_key_t spendSecretKey;

  public_key_t viewPublicKey;
  secret_key_t viewSecretKey;

  void serialize(ISerializer& serializer, const std::string& name);
};

} //namespace cryptonote
