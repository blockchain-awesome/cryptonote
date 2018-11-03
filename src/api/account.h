
#pragma once

#include <string>
#include <cryptonote/core/Account.h>

namespace api
{
struct SecretKeyPair
{
  crypto::secret_key_t view;
  crypto::secret_key_t spend;
};

struct PublicKeyPair
{
  crypto::public_key_t view;
  crypto::public_key_t spend;
};

class Account
{
public:
  Account(std::string spend_key, std::string view_key);
  bool init();

  std::string getAddress()
  {
    return m_address;
  };
  std::string getPublicKeys();
  std::string getPrivateKeys();

  std::string toHex();
  cryptonote::AccountKeys &toKeys();

private:
  std::string &toAdress();

  std::string m_spend_key;
  std::string m_view_key;
  std::string m_address;

  SecretKeyPair m_secret_keys;
  PublicKeyPair m_public_keys;
  cryptonote::AccountKeys m_account_keys;
};

} // namespace api
