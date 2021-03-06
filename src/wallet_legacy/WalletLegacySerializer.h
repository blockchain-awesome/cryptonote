// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>

#include "cryptonote/crypto/hash.h"
#include "cryptonote/crypto/chacha.h"
#include "stream/reader.h"
#include "stream/writer.h"


namespace cryptonote {
class Account;
class ISerializer;
}

namespace cryptonote {

class WalletUserTransactionsCache;

class WalletLegacySerializer {
public:
  WalletLegacySerializer(cryptonote::Account& account, WalletUserTransactionsCache& transactionsCache);

  void serialize(std::ostream& stream, const std::string& password, bool saveDetailed, const std::string& cache);
  void deserialize(std::istream& stream, const std::string& password, std::string& cache);

private:
  void saveKeys(Writer &writer);
  void loadKeys(Reader &reader);

  chacha_iv_t encrypt(const std::string& plain, const std::string& password, std::string& cipher);
  void decrypt(const std::string& cipher, std::string& plain, chacha_iv_t iv, const std::string& password);

  cryptonote::Account& account;
  WalletUserTransactionsCache& transactionsCache;
  const uint32_t walletSerializationVersion;
};

} //namespace cryptonote
