// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "account.h"
#include "cryptonote/core/blockchain/serializer/basics.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "common/base58.h"
#include "common/StringTools.h"

namespace cryptonote
{
//-----------------------------------------------------------------
Account::Account(uint64_t prefix) : m_publicAddressBase58Prefix(prefix)
{
  setNull();
}
//-----------------------------------------------------------------
void Account::setNull()
{
  m_keys = account_keys_t();
}
//-----------------------------------------------------------------
void Account::generate()
{
  crypto::generate_keys(m_keys.address.spendPublicKey, m_keys.spendSecretKey);
  crypto::generate_keys(m_keys.address.viewPublicKey, m_keys.viewSecretKey);
  m_creation_timestamp = time(NULL);
}
//-----------------------------------------------------------------
const account_keys_t &Account::getAccountKeys() const
{
  return m_keys;
}

void Account::setAccountKeys(const account_keys_t &keys)
{
  m_keys = keys;
}
//-----------------------------------------------------------------

void Account::serialize(ISerializer &s)
{
  s(m_keys, "m_keys");
  s(m_creation_timestamp, "m_creation_timestamp");
}

std::string Account::getAddress(const account_public_address_t &adr, uint64_t prefix)
{
  binary_array_t ba;
  bool r = toBinaryArray(adr, ba);
  assert(r);
  return Tools::Base58::encode_addr(prefix, Common::asString(ba));
}

std::string Account::toAddress()
{
  binary_array_t ba;
  bool r = toBinaryArray(m_keys.address, ba);
  assert(r);
  return Tools::Base58::encode_addr(m_publicAddressBase58Prefix, Common::asString(ba));
}

bool Account::parseAddress(uint64_t &prefix, account_public_address_t &adr, const std::string &str)
{
  std::string data;

  return Tools::Base58::decode_addr(str, prefix, data) &&
         fromBinaryArray(adr, Common::asBinaryArray(data)) &&
         // ::serialization::parse_binary(data, adr) &&
         check_key(adr.spendPublicKey) &&
         check_key(adr.viewPublicKey);
}

bool Account::parseAddress(const std::string &str, account_public_address_t &addr, uint64_t prefixExpected)
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

bool Account::parseAddress(const std::string &str, uint64_t prefix)
{
  account_public_address_t addr;
  return Account::parseAddress(str, addr, prefix);
}
} // namespace cryptonote
