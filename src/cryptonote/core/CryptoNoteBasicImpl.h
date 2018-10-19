// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "common/StringTools.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "cryptonote/core/key.h"
#include "cryptonote/core/serialize.h"


namespace cryptonote {
  /************************************************************************/
  /*                                                                      */
  /************************************************************************/
  template<class t_array>
  struct array_hasher: std::unary_function<t_array&, size_t>
  {
    size_t operator()(const t_array& val) const
    {
      return boost::hash_range(&val.data[0], &val.data[sizeof(val.data)]);
    }
  };

  /************************************************************************/
  /* cryptonote.helper functions                                          */
  /************************************************************************/
  uint64_t getPenalizedAmount(uint64_t amount, size_t medianSize, size_t currentBlockSize);
  std::string getAccountAddressAsStr(uint64_t prefix, const AccountPublicAddress& adr);
  bool parseAccountAddressString(uint64_t& prefix, AccountPublicAddress& adr, const std::string& str);
  bool is_coinbase(const Transaction& tx);

  bool operator ==(const cryptonote::Transaction& a, const cryptonote::Transaction& b);
  bool operator ==(const cryptonote::Block& a, const cryptonote::Block& b);
}
