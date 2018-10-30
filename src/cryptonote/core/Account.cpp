// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Account.h"
#include "CryptoNoteSerialization.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "common/base58.h"
#include "common/StringTools.h"

namespace cryptonote
{
//-----------------------------------------------------------------
AccountBase::AccountBase(uint64_t prefix) : m_publicAddressBase58Prefix(prefix)
{
  setNull();
}
//-----------------------------------------------------------------
void AccountBase::setNull()
{
  m_keys = AccountKeys();
}
//-----------------------------------------------------------------
void AccountBase::generate()
{
  crypto::generate_keys(m_keys.address.spendPublicKey, m_keys.spendSecretKey);
  crypto::generate_keys(m_keys.address.viewPublicKey, m_keys.viewSecretKey);
  m_creation_timestamp = time(NULL);
}
//-----------------------------------------------------------------
const AccountKeys &AccountBase::getAccountKeys() const
{
  return m_keys;
}

void AccountBase::setAccountKeys(const AccountKeys &keys)
{
  m_keys = keys;
}
//-----------------------------------------------------------------

void AccountBase::serialize(ISerializer &s)
{
  s(m_keys, "m_keys");
  s(m_creation_timestamp, "m_creation_timestamp");
}

std::string AccountBase::getAddress(const AccountPublicAddress &adr, uint64_t prefix)
{
  BinaryArray ba;
  bool r = toBinaryArray(adr, ba);
  assert(r);
  return Tools::Base58::encode_addr(prefix, Common::asString(ba));
}

std::string AccountBase::toAddress()
{
  BinaryArray ba;
  bool r = toBinaryArray(m_keys.address, ba);
  assert(r);
  return Tools::Base58::encode_addr(m_publicAddressBase58Prefix, Common::asString(ba));
}

bool AccountBase::parseAddress(uint64_t &prefix, AccountPublicAddress &adr, const std::string &str)
{
  std::string data;

  return Tools::Base58::decode_addr(str, prefix, data) &&
         fromBinaryArray(adr, Common::asBinaryArray(data)) &&
         // ::serialization::parse_binary(data, adr) &&
         check_key(adr.spendPublicKey) &&
         check_key(adr.viewPublicKey);
}

bool AccountBase::parseAddress(const std::string &str, AccountPublicAddress &addr, uint64_t prefixExpected)
{
  uint64_t prefix;
  if (!parseAddress(prefix, addr, str))
  {
    return false;
  }

  if (prefix != prefixExpected)
  {
    return false;
  }

  return true;
}

bool AccountBase::parseAddress(const std::string &str, uint64_t prefix)
{
  AccountPublicAddress addr;
  return AccountBase::parseAddress(str, addr, prefix);
}
} // namespace cryptonote
