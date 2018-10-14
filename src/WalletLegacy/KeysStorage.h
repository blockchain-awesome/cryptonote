// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "crypto/crypto.h"

#include <stdint.h>

namespace cryptonote {

class ISerializer;

//This is DTO structure. Do not change it.
struct KeysStorage {
  uint64_t creationTimestamp;

  crypto::PublicKey spendPublicKey;
  crypto::SecretKey spendSecretKey;

  crypto::PublicKey viewPublicKey;
  crypto::SecretKey viewSecretKey;

  void serialize(ISerializer& serializer, const std::string& name);
};

} //namespace cryptonote
