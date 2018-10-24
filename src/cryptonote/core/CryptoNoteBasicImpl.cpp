// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CryptoNoteBasicImpl.h"
#include "CryptoNoteFormatUtils.h"
#include "CryptoNoteTools.h"
#include "CryptoNoteSerialization.h"

#include "common/Base58.h"
#include "crypto/hash.h"
#include "common/int-util.h"

using namespace crypto;
using namespace Common;

namespace cryptonote {

  /************************************************************************/
  /* cryptonote.helper functions                                          */
  /************************************************************************/
  //-----------------------------------------------------------------------------------------------
  uint64_t getPenalizedAmount(uint64_t amount, size_t medianSize, size_t currentBlockSize) {
    static_assert(sizeof(size_t) >= sizeof(uint32_t), "size_t is too small");
    assert(currentBlockSize <= 2 * medianSize);
    assert(medianSize <= std::numeric_limits<uint32_t>::max());
    assert(currentBlockSize <= std::numeric_limits<uint32_t>::max());

    if (amount == 0) {
      return 0;
    }

    if (currentBlockSize <= medianSize) {
      return amount;
    }

    uint64_t productHi;
    uint64_t productLo = mul128(amount, currentBlockSize * (UINT64_C(2) * medianSize - currentBlockSize), &productHi);

    uint64_t penalizedAmountHi;
    uint64_t penalizedAmountLo;
    div128_32(productHi, productLo, static_cast<uint32_t>(medianSize), &penalizedAmountHi, &penalizedAmountLo);
    div128_32(penalizedAmountHi, penalizedAmountLo, static_cast<uint32_t>(medianSize), &penalizedAmountHi, &penalizedAmountLo);

    assert(0 == penalizedAmountHi);
    assert(penalizedAmountLo < amount);

    return penalizedAmountLo;
  }

  //-----------------------------------------------------------------------
  bool operator ==(const cryptonote::Transaction& a, const cryptonote::Transaction& b) {
    return getObjectHash(a) == getObjectHash(b);
  }
  //-----------------------------------------------------------------------
  bool operator ==(const cryptonote::Block& a, const cryptonote::Block& b) {
    return cryptonote::get_block_hash(a) == cryptonote::get_block_hash(b);
  }
}

//--------------------------------------------------------------------------------
bool parse_hash256(const std::string& str_hash, crypto::Hash& hash) {
  return Common::podFromHex(str_hash, hash);
}
