#include "WalletFile.h"

#include <fstream>

#include "crypto/chacha.h"
#include "CryptoNoteConfig.h"
#include "WalletLegacy/KeysStorage.h"
#include "common/StringTools.h"
#include "common/StdOutputStream.h"
#include "common/Base58.h"
#include "cryptonote_core/CryptoNoteBasicImpl.h"
#include "cryptonote_core/Currency.h"

#include "Serialization/BinaryOutputStreamSerializer.h"
#include "cryptonote_core/CryptoNoteSerialization.h"

using namespace Common;
using namespace cryptonote;

 bool create_wallet_by_keys(std::string &wallet_file, std::string &password,
  std::string &address, std::string &spendKey, std::string &viewKey,
  Logging::LoggerRef &logger) {
  crypto::SecretKey viewSecretKey;
  crypto::SecretKey sendSecretKey;
  if (!Common::podFromHex(viewKey, viewSecretKey))
  {
    logger(Logging::ERROR) << "Cannot parse view secret key: " << viewKey;
    return false;
  }
  if (!Common::podFromHex(spendKey, sendSecretKey))
  {
    logger(Logging::ERROR) << "Cannot parse send secret key: " << spendKey;
    return false;
  }

  crypto::PublicKey sendPubKey;
  crypto::PublicKey viewPubKey;

  if (!crypto::secret_key_to_public_key(sendSecretKey, sendPubKey))
  {
    logger(Logging::ERROR) << "Cannot get send public key from secret key: " << spendKey;
    return false;
  }

  if (!crypto::secret_key_to_public_key(viewSecretKey, viewPubKey))
  {
    logger(Logging::ERROR) << "Cannot get send public key from secret key: " << viewKey;
    return false;
  }

  char keyStore[sizeof(crypto::PublicKey) * 2];

  memcpy(keyStore, &sendPubKey, sizeof(crypto::PublicKey));
  memcpy(keyStore + sizeof(crypto::PublicKey), &viewPubKey, sizeof(crypto::PublicKey));

  using namespace parameters;
  const uint64_t prefix = CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;
  std::string addressStr = Tools::Base58::encode_addr(prefix, std::string(keyStore, sizeof(crypto::PublicKey) * 2));
  if (addressStr != address)
  {
    logger(Logging::ERROR) << "Addresses mismatch!" << addressStr;
    return false;
  }

  logger(Logging::INFO) << "Addresses match!";

  cryptonote::AccountKeys keys;
  keys.address.spendPublicKey = sendPubKey;
  keys.address.viewPublicKey = viewPubKey;
  keys.spendSecretKey = sendSecretKey;
  keys.viewSecretKey = viewSecretKey;

  try
  {
    std::ofstream file;
    try
    {
      file.open(wallet_file, std::ios_base::binary | std::ios_base::out | std::ios::trunc);
      if (file.fail())
      {
        throw std::runtime_error("error opening file: " + wallet_file);
      }
    }
    catch (std::exception &e)
    {
      logger(Logging::INFO) << "exception !" << e.what();
    }
    std::stringstream plainArchive;
    StdOutputStream plainStream(plainArchive);
    cryptonote::BinaryOutputStreamSerializer serializer(plainStream);

    // Saving Keys;

    cryptonote::KeysStorage ks;

    ks.creationTimestamp = time(NULL);
    ks.spendPublicKey = keys.address.spendPublicKey;
    ks.spendSecretKey = keys.spendSecretKey;
    ks.viewPublicKey = keys.address.viewPublicKey;
    ks.viewSecretKey = keys.viewSecretKey;

    ks.serialize(serializer, "keys");

    bool hasDetails = false;

    serializer(hasDetails, "has_details");

    std::string cache("");

    serializer.binary(const_cast<std::string &>(cache), "cache");

    std::string plain = plainArchive.str();
    std::string cipher;
    crypto::chacha_key key;
    crypto::generate_chacha_key(password, key);

    cipher.resize(plain.size());

    crypto::chacha_iv iv = crypto::rand<crypto::chacha_iv>();
    crypto::chacha8(plain.data(), plain.size(), key, iv, &cipher[0]);

    uint32_t version = 1;
    StdOutputStream output(file);
    cryptonote::BinaryOutputStreamSerializer s(output);
    s.beginObject("wallet");
    s(version, "version");
    s(iv, "iv");
    s(cipher, "data");
    s.endObject();

    file.flush();
    file.close();
    logger(Logging::INFO, Logging::BRIGHT_WHITE) << "Generated new wallet: " << address << std::endl
                               << "view key: " << Common::podToHex(keys.viewSecretKey);
  }
  catch (const std::exception &e)
  {
    logger(Logging::INFO, Logging::BRIGHT_WHITE) << "failed to generate new wallet: " << e.what();
    return false;
  }
  return true;
}