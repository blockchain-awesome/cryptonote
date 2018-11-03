#include "account.h"

// #include <fstream>

// #include "crypto/chacha.h"
#include "CryptoNoteConfig.h"
// #include "wallet_legacy/KeysStorage.h"
// #include "common/StringTools.h"
// #include "stream/StdOutputStream.h"
#include "common/base58.h"
#include "cryptonote/core/CryptoNoteBasicImpl.h"
#include "cryptonote/core/Currency.h"

// #include "serialization/BinaryOutputStreamSerializer.h"
// #include "cryptonote/core/blockchain/serializer/basics.h"

// using namespace Common;
// using namespace cryptonote;

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
  char keyStore[sizeof(crypto::public_key_t) * 2];

  memcpy(keyStore, &m_public_keys.spend, sizeof(crypto::public_key_t));
  memcpy(keyStore + sizeof(crypto::public_key_t), &m_public_keys.view, sizeof(crypto::public_key_t));
  const uint64_t prefix = cryptonote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;
  m_address = Tools::Base58::encode_addr(prefix, std::string(keyStore, sizeof(crypto::public_key_t) * 2));
  return m_address;
}

cryptonote::AccountKeys &Account::toKeys()
{
  cryptonote::AccountKeys &keys = m_account_keys;
  keys.address.spendPublicKey = m_public_keys.spend;
  keys.address.viewPublicKey = m_public_keys.view;
  keys.spendSecretKey = m_secret_keys.spend;
  keys.viewSecretKey = m_secret_keys.view;

  return m_account_keys;
}

} // namespace api