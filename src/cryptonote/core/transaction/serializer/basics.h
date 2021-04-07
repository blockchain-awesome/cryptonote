// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote/core/key.h"
#include "common/int-util.h"
#include "cryptonote/crypto/chacha.h"
#include "serialization/ISerializer.h"
#include "cryptonote/crypto/crypto.h"
#include "cryptonote/core/blockchain/serializer/crypto.h"
#include "cryptonote/types.h"

namespace cryptonote
{

  struct account_keys_t;

  void serialize(transaction_prefix_t &txP, ISerializer &serializer);
  void serialize(transaction_t &tx, ISerializer &serializer);
  void serialize(transaction_input_t &in, ISerializer &serializer);

  void serialize(base_input_t &gen, ISerializer &serializer);
  void serialize(key_input_t &key, ISerializer &serializer);
  void serialize(multi_signature_input_t &multisignature, ISerializer &serializer);

  void serialize(transaction_output_t &output, ISerializer &serializer);
  void serialize(transaction_output_target_t &output, ISerializer &serializer);
  void serialize(key_output_t &key, ISerializer &serializer);
  void serialize(multi_signature_output_t &multisignature, ISerializer &serializer);
  void serialize(transaction_index_t &idx, ISerializer &serializer);

  void serialize(block_header_t &header, ISerializer &serializer);
  void serialize(block_t &block, ISerializer &serializer);

  void serialize(account_public_address_t &address, ISerializer &serializer);
  void serialize(account_keys_t &keys, ISerializer &s);

  void serialize(key_pair_t &keyPair, ISerializer &serializer);

  // For block explorer

  void serialize(transaction_input_generate_details_t &tigd, ISerializer &serializer);
  void serialize(transaction_input_to_key_details_t &titkd, ISerializer &serializer);
  void serialize(transaction_input_multisignature_details_t &timd, ISerializer &serializer);
  void serialize(transaction_input_details_base_t &tid, ISerializer &serializer);
  void serialize(transaction_input_details_t &tid, ISerializer &serializer);


  struct transaction_priority_comparator_t
  {
    // lhs > hrs
    bool operator()(const transaction_details_t &lhs, const transaction_details_t &rhs) const
    {
      // price(lhs) = lhs.fee / lhs.blobSize
      // price(lhs) > price(rhs) -->
      // lhs.fee / lhs.blobSize > rhs.fee / rhs.blobSize -->
      // lhs.fee * rhs.blobSize > rhs.fee * lhs.blobSize
      uint64_t lhs_hi, lhs_lo = mul128(lhs.fee, rhs.blobSize, &lhs_hi);
      uint64_t rhs_hi, rhs_lo = mul128(rhs.fee, lhs.blobSize, &rhs_hi);

      return
          // prefer more profitable transactions
          (lhs_hi > rhs_hi) ||
          (lhs_hi == rhs_hi && lhs_lo > rhs_lo) ||
          // prefer smaller
          (lhs_hi == rhs_hi && lhs_lo == rhs_lo && lhs.blobSize < rhs.blobSize) ||
          // prefer older
          (lhs_hi == rhs_hi && lhs_lo == rhs_lo && lhs.blobSize == rhs.blobSize && lhs.receiveTime < rhs.receiveTime);
    }
  };

  void serialize(transaction_details_t &td, ISerializer &s);

}
