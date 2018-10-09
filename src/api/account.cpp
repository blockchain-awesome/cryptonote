#include "account.h"

// #include <fstream>

// #include "crypto/chacha.h"
#include "CryptoNoteConfig.h"
// #include "WalletLegacy/KeysStorage.h"
// #include "common/StringTools.h"
// #include "common/StdOutputStream.h"
#include "common/Base58.h"
#include "cryptonote_core/CryptoNoteBasicImpl.h"
#include "cryptonote_core/Currency.h"

// #include "Serialization/BinaryOutputStreamSerializer.h"
// #include "cryptonote_core/CryptoNoteSerialization.h"

// using namespace Common;
// using namespace CryptoNote;

namespace api
{

Account::Account(std::string spend_key, std::string view_key)
    : m_view_key(view_key), m_spend_key(spend_key)
{
}
bool Account::init()
{
  if (!Common::podFromHex(m_view_key, m_secret_keys.view))
  {
    return false;
  }

  if (!Common::podFromHex(m_spend_key, m_secret_keys.spend))
  {
    return false;
  }

  if (!crypto::secret_key_to_public_key(m_secret_keys.spend, m_public_keys.spend))
  {
    return false;
  }

  if (!crypto::secret_key_to_public_key(m_secret_keys.view, m_public_keys.view))
  {
    return false;
  }
  return true;
}

std::string &Account::toAdress()
{
  char keyStore[sizeof(crypto::PublicKey) * 2];

  memcpy(keyStore, &m_public_keys.spend, sizeof(crypto::PublicKey));
  memcpy(keyStore + sizeof(crypto::PublicKey), &m_public_keys.view, sizeof(crypto::PublicKey));
  const uint64_t prefix = CryptoNote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;
  m_address = Tools::Base58::encode_addr(prefix, std::string(keyStore, sizeof(crypto::PublicKey) * 2));
  return m_address;
}

CryptoNote::AccountKeys &Account::toKeys()
{
  CryptoNote::AccountKeys &keys = m_account_keys;
  keys.address.spendPublicKey = m_public_keys.spend;
  keys.address.viewPublicKey = m_public_keys.view;
  keys.spendSecretKey = m_secret_keys.spend;
  keys.viewSecretKey = m_secret_keys.view;

  return m_account_keys;
}

} // namespace api