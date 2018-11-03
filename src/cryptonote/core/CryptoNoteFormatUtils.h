// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <boost/utility/value_init.hpp>

#include "key.h"
#include "cryptonote/core/blockchain/serializer/basics.h"

#include "serialization/BinaryOutputStreamSerializer.h"
#include "serialization/BinaryInputStreamSerializer.h"

namespace Logging {
class ILogger;
}

namespace cryptonote {

bool parseAndValidateTransactionFromBinaryArray(const BinaryArray& transactionBinaryArray, Transaction& transaction, crypto::Hash& transactionHash, crypto::Hash& transactionPrefixHash);

struct TransactionSourceEntry {
  typedef std::pair<uint32_t, crypto::public_key_t> OutputEntry;

  std::vector<OutputEntry> outputs;           //index + key
  size_t realOutput;                          //index in outputs vector of real output_entry
  crypto::public_key_t realTransactionPublicKey; //incoming real tx public key
  size_t realOutputIndexInTransaction;        //index in transaction outputs vector
  uint64_t amount;                            //money
};

struct TransactionDestinationEntry {
  uint64_t amount;                    //money
  AccountPublicAddress addr;          //destination address

  TransactionDestinationEntry() : amount(0), addr(boost::value_initialized<AccountPublicAddress>()) {}
  TransactionDestinationEntry(uint64_t amount, const AccountPublicAddress &addr) : amount(amount), addr(addr) {}
};


bool constructTransaction(
  const AccountKeys& senderAccountKeys,
  const std::vector<TransactionSourceEntry>& sources,
  const std::vector<TransactionDestinationEntry>& destinations,
  std::vector<uint8_t> extra, Transaction& transaction, uint64_t unlock_time, Logging::ILogger& log);


bool is_out_to_acc(const AccountKeys& acc, const KeyOutput& out_key, const crypto::public_key_t& tx_pub_key, size_t keyIndex);
bool is_out_to_acc(const AccountKeys& acc, const KeyOutput& out_key, const crypto::key_derivation_t& derivation, size_t keyIndex);
bool lookup_acc_outs(const AccountKeys& acc, const Transaction& tx, const crypto::public_key_t& tx_pub_key, std::vector<size_t>& outs, uint64_t& money_transfered);
bool lookup_acc_outs(const AccountKeys& acc, const Transaction& tx, std::vector<size_t>& outs, uint64_t& money_transfered);
bool get_tx_fee(const Transaction& tx, uint64_t & fee);
uint64_t get_tx_fee(const Transaction& tx);
bool generate_key_image_helper(const AccountKeys& ack, const crypto::public_key_t& tx_public_key, size_t real_output_index, KeyPair& in_ephemeral, crypto::key_image_t& ki);
std::string short_hash_str(const crypto::Hash& h);

bool get_block_hashing_blob(const block_t& b, BinaryArray& blob);
bool get_aux_block_header_hash(const block_t& b, crypto::Hash& res);
bool get_block_hash(const block_t& b, crypto::Hash& res);
crypto::Hash get_block_hash(const block_t& b);
bool get_block_longhash(const block_t& b, crypto::Hash& res);
bool get_inputs_money_amount(const Transaction& tx, uint64_t& money);
uint64_t get_outs_money_amount(const Transaction& tx);
bool check_inputs_types_supported(const TransactionPrefix& tx);
bool check_outs_valid(const TransactionPrefix& tx, std::string* error = 0);
bool checkMultisignatureInputsDiff(const TransactionPrefix& tx);

bool check_money_overflow(const TransactionPrefix& tx);
bool check_outs_overflow(const TransactionPrefix& tx);
bool check_inputs_overflow(const TransactionPrefix& tx);
uint32_t get_block_height(const block_t& b);
std::vector<uint32_t> relative_output_offsets_to_absolute(const std::vector<uint32_t>& off);
std::vector<uint32_t> absolute_output_offsets_to_relative(const std::vector<uint32_t>& off);


// 62387455827 -> 455827 + 7000000 + 80000000 + 300000000 + 2000000000 + 60000000000, where 455827 <= dust_threshold
template<typename chunk_handler_t, typename dust_handler_t>
void decompose_amount_into_digits(uint64_t amount, uint64_t dust_threshold, const chunk_handler_t& chunk_handler, const dust_handler_t& dust_handler) {
  if (0 == amount) {
    return;
  }

  bool is_dust_handled = false;
  uint64_t dust = 0;
  uint64_t order = 1;
  while (0 != amount) {
    uint64_t chunk = (amount % 10) * order;
    amount /= 10;
    order *= 10;

    if (dust + chunk <= dust_threshold) {
      dust += chunk;
    } else {
      if (!is_dust_handled && 0 != dust) {
        dust_handler(dust);
        is_dust_handled = true;
      }
      if (0 != chunk) {
        chunk_handler(chunk);
      }
    }
  }

  if (!is_dust_handled && 0 != dust) {
    dust_handler(dust);
  }
}

void get_tx_tree_hash(const std::vector<crypto::Hash>& tx_hashes, crypto::Hash& h);
crypto::Hash get_tx_tree_hash(const std::vector<crypto::Hash>& tx_hashes);
crypto::Hash get_tx_tree_hash(const block_t& b);

}
