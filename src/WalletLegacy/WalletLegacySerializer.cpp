// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "WalletLegacySerializer.h"

#include <stdexcept>

#include "stream/MemoryInputStream.h"
#include "stream/StdInputStream.h"
#include "stream/StdOutputStream.h"
#include "Serialization/BinaryOutputStreamSerializer.h"
#include "Serialization/BinaryInputStreamSerializer.h"
#include "cryptonote/core/Account.h"
#include "cryptonote/core/CryptoNoteSerialization.h"
#include "WalletLegacy/WalletUserTransactionsCache.h"
#include "Wallet/WalletErrors.h"
#include "WalletLegacy/KeysStorage.h"

using namespace Common;

namespace {

bool verifyKeys(const crypto::SecretKey& sec, const crypto::PublicKey& expected_pub) {
  crypto::PublicKey pub;
  bool r = crypto::secret_key_to_public_key(sec, pub);
  return r && expected_pub == pub;
}

void throwIfKeysMissmatch(const crypto::SecretKey& sec, const crypto::PublicKey& expected_pub) {
  if (!verifyKeys(sec, expected_pub))
    throw std::system_error(make_error_code(cryptonote::error::WRONG_PASSWORD));
}

}

namespace cryptonote {

WalletLegacySerializer::WalletLegacySerializer(cryptonote::AccountBase& account, WalletUserTransactionsCache& transactionsCache) :
  account(account),
  transactionsCache(transactionsCache),
  walletSerializationVersion(1)
{
}

void WalletLegacySerializer::serialize(std::ostream& stream, const std::string& password, bool saveDetailed, const std::string& cache) {
  std::stringstream plainArchive;
  StdOutputStream plainStream(plainArchive);
  cryptonote::BinaryOutputStreamSerializer serializer(plainStream);
  saveKeys(serializer);

  serializer(saveDetailed, "has_details");

  if (saveDetailed) {
    serializer(transactionsCache, "details");
  }

  serializer.binary(const_cast<std::string&>(cache), "cache");

  std::string plain = plainArchive.str();
  std::string cipher;

  crypto::chacha_iv iv = encrypt(plain, password, cipher);

  uint32_t version = walletSerializationVersion;
  StdOutputStream output(stream);
  cryptonote::BinaryOutputStreamSerializer s(output);
  s.beginObject("wallet");
  s(version, "version");
  s(iv, "iv");
  s(cipher, "data");
  s.endObject();

  stream.flush();
}

void WalletLegacySerializer::saveKeys(cryptonote::ISerializer& serializer) {
  cryptonote::KeysStorage keys;
  cryptonote::AccountKeys acc = account.getAccountKeys();

  keys.creationTimestamp = account.get_createtime();
  keys.spendPublicKey = acc.address.spendPublicKey;
  keys.spendSecretKey = acc.spendSecretKey;
  keys.viewPublicKey = acc.address.viewPublicKey;
  keys.viewSecretKey = acc.viewSecretKey;

  keys.serialize(serializer, "keys");
}

crypto::chacha_iv WalletLegacySerializer::encrypt(const std::string& plain, const std::string& password, std::string& cipher) {
  crypto::chacha_key key;
  crypto::generate_chacha_key(password, key);

  cipher.resize(plain.size());

  crypto::chacha_iv iv = crypto::rand<crypto::chacha_iv>();
  crypto::chacha8(plain.data(), plain.size(), key, iv, &cipher[0]);

  return iv;
}


void WalletLegacySerializer::deserialize(std::istream& stream, const std::string& password, std::string& cache) {
  StdInputStream stdStream(stream);
  cryptonote::BinaryInputStreamSerializer serializerEncrypted(stdStream);

  serializerEncrypted.beginObject("wallet");

  uint32_t version;
  serializerEncrypted(version, "version");

  crypto::chacha_iv iv;
  serializerEncrypted(iv, "iv");

  std::string cipher;
  serializerEncrypted(cipher, "data");

  serializerEncrypted.endObject();

  std::string plain;
  decrypt(cipher, plain, iv, password);

  MemoryInputStream decryptedStream(plain.data(), plain.size()); 
  cryptonote::BinaryInputStreamSerializer serializer(decryptedStream);

  loadKeys(serializer);
  throwIfKeysMissmatch(account.getAccountKeys().viewSecretKey, account.getAccountKeys().address.viewPublicKey);

  if (account.getAccountKeys().spendSecretKey != NULL_SECRET_KEY) {
    throwIfKeysMissmatch(account.getAccountKeys().spendSecretKey, account.getAccountKeys().address.spendPublicKey);
  } else {
    if (!crypto::check_key(account.getAccountKeys().address.spendPublicKey)) {
      throw std::system_error(make_error_code(cryptonote::error::WRONG_PASSWORD));
    }
  }

  bool detailsSaved;

  serializer(detailsSaved, "has_details");

  if (detailsSaved) {
    serializer(transactionsCache, "details");
  }

  serializer.binary(cache, "cache");
}

void WalletLegacySerializer::decrypt(const std::string& cipher, std::string& plain, crypto::chacha_iv iv, const std::string& password) {
  crypto::chacha_key key;
  crypto::generate_chacha_key(password, key);

  plain.resize(cipher.size());

  crypto::chacha8(cipher.data(), cipher.size(), key, iv, &plain[0]);
}

void WalletLegacySerializer::loadKeys(cryptonote::ISerializer& serializer) {
  cryptonote::KeysStorage keys;

  keys.serialize(serializer, "keys");

  cryptonote::AccountKeys acc;
  acc.address.spendPublicKey = keys.spendPublicKey;
  acc.spendSecretKey = keys.spendSecretKey;
  acc.address.viewPublicKey = keys.viewPublicKey;
  acc.viewSecretKey = keys.viewSecretKey;

  account.setAccountKeys(acc);
  account.set_createtime(keys.creationTimestamp);
}

}
