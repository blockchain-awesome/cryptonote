// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote/core/key.h"
#include "crypto/crypto.h"
#include "CryptoNoteConfig.h"

namespace cryptonote
{

class ISerializer;

/************************************************************************/
/*                                                                      */
/************************************************************************/
class AccountBase
{
public:
  AccountBase(uint64_t prefix = parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX);
  void generate();

  const AccountKeys &getAccountKeys() const;
  void setAccountKeys(const AccountKeys &keys);

  static bool parseAddress(uint64_t &prefix, AccountPublicAddress &adr, const std::string &str);
  static bool parseAddress(const std::string &str, AccountPublicAddress &addr, uint64_t prefixExpected = parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX);
  static bool parseAddress(const std::string &str, uint64_t prefix = parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX);

  static std::string getAddress(const AccountPublicAddress &adr, uint64_t prefix = parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX);
  std::string toAddress();

  uint64_t getCreatetime() const { return m_creation_timestamp; }
  void setCreatetime(uint64_t val) { m_creation_timestamp = val; }
  void serialize(ISerializer &s);

  template <class t_archive>
  inline void serialize(t_archive &a, const unsigned int /*ver*/)
  {
    a &m_keys;
    a &m_creation_timestamp;
  }

private:
  void setNull();
  AccountKeys m_keys;
  uint64_t m_creation_timestamp;
  uint64_t m_publicAddressBase58Prefix;
};

typedef AccountBase Account;
} // namespace cryptonote
