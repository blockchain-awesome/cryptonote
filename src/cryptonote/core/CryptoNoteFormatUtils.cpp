// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CryptoNoteFormatUtils.h"

#include <set>
#include <logging/LoggerRef.h>
#include <common/varint.h>

#include "serialization/BinaryOutputStreamSerializer.h"
#include "serialization/BinaryInputStreamSerializer.h"

#include "account.h"
#include "TransactionExtra.h"
#include "CryptoNoteTools.h"

#include "CryptoNoteConfig.h"
#include "cryptonote/structures/array.hpp"

using namespace Logging;
using namespace crypto;
using namespace Common;

extern "C" {
  extern void cn_fast_hash(const void *data, size_t length, char *hash);
}

namespace cryptonote {

bool parseAndValidateTransactionFromBinaryArray(const binary_array_t& tx_blob, transaction_t& tx, hash_t& tx_hash, hash_t& tx_prefix_hash) {
  if (!BinaryArray::from(tx, tx_blob)) {
    return false;
  }

  //TODO: validate tx
  cn_fast_hash(tx_blob.data(), tx_blob.size(), (char *)&tx_hash);
  BinaryArray::objectHash(*static_cast<transaction_prefix_t*>(&tx), tx_prefix_hash);
  return true;
}

bool generate_key_image_helper(const account_keys_t& ack, const public_key_t& tx_public_key, size_t real_output_index, key_pair_t& in_ephemeral, key_image_t& ki) {
  key_derivation_t recv_derivation;
  bool r = generate_key_derivation((const uint8_t*)&tx_public_key, (const uint8_t*)&ack.viewSecretKey, (uint8_t*)&recv_derivation);

  assert(r && "key image helper: failed to generate_key_derivation");

  if (!r) {
    return false;
  }

  r = derive_public_key((const uint8_t*)&recv_derivation, real_output_index, (const uint8_t*)&ack.address.spendPublicKey, (uint8_t*)&in_ephemeral.publicKey);

  assert(r && "key image helper: failed to derive_public_key");

  if (!r) {
    return false;
  }

  derive_secret_key((const uint8_t*)&recv_derivation, real_output_index, (const uint8_t*)&ack.spendSecretKey, (uint8_t*)&in_ephemeral.secretKey);
  generate_key_image((const uint8_t *)&in_ephemeral.publicKey, (const uint8_t *)&in_ephemeral.secretKey, (uint8_t *)&ki);
  return true;
}

uint64_t power_integral(uint64_t a, uint64_t b) {
  if (b == 0)
    return 1;
  uint64_t total = a;
  for (uint64_t i = 1; i != b; i++)
    total *= a;
  return total;
}

bool get_tx_fee(const transaction_t& tx, uint64_t & fee) {
  uint64_t amount_in = 0;
  uint64_t amount_out = 0;

  for (const auto& in : tx.inputs) {
    if (in.type() == typeid(key_input_t)) {
      amount_in += boost::get<key_input_t>(in).amount;
    } else if (in.type() == typeid(multi_signature_input_t)) {
      amount_in += boost::get<multi_signature_input_t>(in).amount;
    }
  }

  for (const auto& o : tx.outputs) {
    amount_out += o.amount;
  }

  if (!(amount_in >= amount_out)) {
    return false;
  }

  fee = amount_in - amount_out;
  return true;
}

uint64_t get_tx_fee(const transaction_t& tx) {
  uint64_t r = 0;
  if (!get_tx_fee(tx, r))
    return 0;
  return r;
}


bool constructTransaction(
  const account_keys_t& sender_account_keys,
  const std::vector<transaction_source_entry_t>& sources,
  const std::vector<transaction_destination_entry_t>& destinations,
  std::vector<uint8_t> extra,
  transaction_t& tx,
  uint64_t unlock_time,
  Logging::ILogger& log) {
  LoggerRef logger(log, "construct_tx");

  tx.inputs.clear();
  tx.outputs.clear();
  tx.signatures.clear();

  tx.version = config::get().transaction.version.major;
  tx.unlockTime = unlock_time;

  tx.extra = extra;
  // key_pair_t txkey = generateKeyPair();
  key_pair_t txkey = Key::generate();

  addTransactionPublicKeyToExtra(tx.extra, txkey.publicKey);

  struct input_generation_context_data {
    key_pair_t in_ephemeral;
  };

  std::vector<input_generation_context_data> in_contexts;
  uint64_t summary_inputs_money = 0;
  //fill inputs
  for (const transaction_source_entry_t& src_entr : sources) {
    if (src_entr.realOutput >= src_entr.outputs.size()) {
      logger(ERROR) << "real_output index (" << src_entr.realOutput << ")bigger than output_keys.size()=" << src_entr.outputs.size();
      return false;
    }
    summary_inputs_money += src_entr.amount;

    //KeyDerivation recv_derivation;
    in_contexts.push_back(input_generation_context_data());
    key_pair_t& in_ephemeral = in_contexts.back().in_ephemeral;
    key_image_t img;
    if (!generate_key_image_helper(sender_account_keys, src_entr.realTransactionPublicKey, src_entr.realOutputIndexInTransaction, in_ephemeral, img))
      return false;

    //check that derived key is equal with real output key
    if (!(in_ephemeral.publicKey == src_entr.outputs[src_entr.realOutput].second)) {
      logger(ERROR) << "derived public key mismatch with output public key! " << ENDL << "derived_key:"
        << hex::podToString(in_ephemeral.publicKey) << ENDL << "real output_public_key:"
        << hex::podToString(src_entr.outputs[src_entr.realOutput].second);
      return false;
    }

    //put key image into tx input
    key_input_t input_to_key;
    input_to_key.amount = src_entr.amount;
    input_to_key.keyImage = img;

    //fill outputs array and use relative offsets
    for (const transaction_source_entry_t::output_entry_t& out_entry : src_entr.outputs) {
      input_to_key.outputIndexes.push_back(out_entry.first);
    }

    input_to_key.outputIndexes = absolute_output_offsets_to_relative(input_to_key.outputIndexes);
    tx.inputs.push_back(input_to_key);
  }

  // "Shuffle" outs
  std::vector<transaction_destination_entry_t> shuffled_dsts(destinations);
  std::sort(shuffled_dsts.begin(), shuffled_dsts.end(), [](const transaction_destination_entry_t& de1, const transaction_destination_entry_t& de2) { return de1.amount < de2.amount; });

  uint64_t summary_outs_money = 0;
  //fill outputs
  size_t output_index = 0;
  for (const transaction_destination_entry_t& dst_entr : shuffled_dsts) {
    if (!(dst_entr.amount > 0)) {
      logger(ERROR, BRIGHT_RED) << "Destination with wrong amount: " << dst_entr.amount;
      return false;
    }
    key_derivation_t derivation;
    public_key_t out_eph_public_key;
    bool r = generate_key_derivation((const uint8_t*)&dst_entr.addr.viewPublicKey, (const uint8_t*)&txkey.secretKey, (uint8_t*)&derivation);

    if (!(r)) {
      logger(ERROR, BRIGHT_RED)
        << "at creation outs: failed to generate_key_derivation("
        << dst_entr.addr.viewPublicKey << ", " << txkey.secretKey << ")";
      return false;
    }

    r = derive_public_key((const uint8_t*)&derivation, output_index,
      (const uint8_t*)&dst_entr.addr.spendPublicKey,
      (uint8_t*)&out_eph_public_key);
    if (!(r)) {
      logger(ERROR, BRIGHT_RED)
        << "at creation outs: failed to derive_public_key(" << derivation
        << ", " << output_index << ", " << dst_entr.addr.spendPublicKey
        << ")";
      return false;
    }

    transaction_output_t out;
    out.amount = dst_entr.amount;
    key_output_t tk;
    tk.key = out_eph_public_key;
    out.target = tk;
    tx.outputs.push_back(out);
    output_index++;
    summary_outs_money += dst_entr.amount;
  }

  //check money
  if (summary_outs_money > summary_inputs_money) {
    logger(ERROR) << "transaction_t inputs money (" << summary_inputs_money << ") less than outputs money (" << summary_outs_money << ")";
    return false;
  }

  //generate ring signatures
  hash_t tx_prefix_hash;
  BinaryArray::objectHash(*static_cast<transaction_prefix_t*>(&tx), tx_prefix_hash);

  size_t i = 0;
  for (const transaction_source_entry_t& src_entr : sources) {
    std::vector<const public_key_t*> keys_ptrs;
    for (const transaction_source_entry_t::output_entry_t& o : src_entr.outputs) {
      keys_ptrs.push_back(&o.second);
    }

    tx.signatures.push_back(std::vector<signature_t>());
    std::vector<signature_t>& sigs = tx.signatures.back();
    sigs.resize(src_entr.outputs.size());
    generate_ring_signature(tx_prefix_hash, boost::get<key_input_t>(tx.inputs[i]).keyImage, keys_ptrs.data(), keys_ptrs.size(),
      in_contexts[i].in_ephemeral.secretKey, src_entr.realOutput, sigs.data());
    i++;
  }

  return true;
}

bool get_inputs_money_amount(const transaction_t& tx, uint64_t& money) {
  money = 0;

  for (const auto& in : tx.inputs) {
    uint64_t amount = 0;

    if (in.type() == typeid(key_input_t)) {
      amount = boost::get<key_input_t>(in).amount;
    } else if (in.type() == typeid(multi_signature_input_t)) {
      amount = boost::get<multi_signature_input_t>(in).amount;
    }

    money += amount;
  }
  return true;
}

uint32_t get_block_height(const block_t& b) {
  if (b.baseTransaction.inputs.size() != 1) {
    return 0;
  }
  const auto& in = b.baseTransaction.inputs[0];
  if (in.type() != typeid(base_input_t)) {
    return 0;
  }
  return boost::get<base_input_t>(in).blockIndex;
}

bool check_inputs_types_supported(const transaction_prefix_t& tx) {
  for (const auto& in : tx.inputs) {
    if (in.type() != typeid(key_input_t) && in.type() != typeid(multi_signature_input_t)) {
      return false;
    }
  }

  return true;
}

bool check_outs_valid(const transaction_prefix_t& tx, std::string* error) {
  for (const transaction_output_t& out : tx.outputs) {
    if (out.target.type() == typeid(key_output_t)) {
      if (out.amount == 0) {
        if (error) {
          *error = "Zero amount ouput";
        }
        return false;
      }

      if (!check_key((uint8_t *)&boost::get<key_output_t>(out.target).key)) {
        if (error) {
          *error = "Output with invalid key";
        }
        return false;
      }
    } else if (out.target.type() == typeid(multi_signature_output_t)) {
      const multi_signature_output_t& multisignatureOutput = ::boost::get<multi_signature_output_t>(out.target);
      if (multisignatureOutput.requiredSignatureCount > multisignatureOutput.keys.size()) {
        if (error) {
          *error = "Multisignature output with invalid required signature count";
        }
        return false;
      }
      for (const public_key_t& key : multisignatureOutput.keys) {
        if (!check_key((uint8_t *)&key)) {
          if (error) {
            *error = "Multisignature output with invalid public key";
          }
          return false;
        }
      }
    } else {
      if (error) {
        *error = "Output with invalid type";
      }
      return false;
    }
  }

  return true;
}

bool checkMultisignatureInputsDiff(const transaction_prefix_t& tx) {
  std::set<std::pair<uint64_t, uint32_t>> inputsUsage;
  for (const auto& inv : tx.inputs) {
    if (inv.type() == typeid(multi_signature_input_t)) {
      const multi_signature_input_t& in = ::boost::get<multi_signature_input_t>(inv);
      if (!inputsUsage.insert(std::make_pair(in.amount, in.outputIndex)).second) {
        return false;
      }
    }
  }
  return true;
}

bool check_money_overflow(const transaction_prefix_t &tx) {
  return check_inputs_overflow(tx) && check_outs_overflow(tx);
}

bool check_inputs_overflow(const transaction_prefix_t &tx) {
  uint64_t money = 0;

  for (const auto &in : tx.inputs) {
    uint64_t amount = 0;

    if (in.type() == typeid(key_input_t)) {
      amount = boost::get<key_input_t>(in).amount;
    } else if (in.type() == typeid(multi_signature_input_t)) {
      amount = boost::get<multi_signature_input_t>(in).amount;
    }

    if (money > amount + money)
      return false;

    money += amount;
  }
  return true;
}

bool check_outs_overflow(const transaction_prefix_t& tx) {
  uint64_t money = 0;
  for (const auto& o : tx.outputs) {
    if (money > o.amount + money)
      return false;
    money += o.amount;
  }
  return true;
}

uint64_t get_outs_money_amount(const transaction_t& tx) {
  uint64_t outputs_amount = 0;
  for (const auto& o : tx.outputs) {
    outputs_amount += o.amount;
  }
  return outputs_amount;
}

std::string short_hash_str(const hash_t& h) {
  std::string res = hex::podToString(h);

  if (res.size() == 64) {
    auto erased_pos = res.erase(8, 48);
    res.insert(8, "....");
  }

  return res;
}

bool is_out_to_acc(const account_keys_t& acc, const key_output_t& out_key, const key_derivation_t& derivation, size_t keyIndex) {
  public_key_t pk;
  derive_public_key((const uint8_t*)&derivation, keyIndex, (const uint8_t*)&acc.address.spendPublicKey, (uint8_t*)&pk);
  return pk == out_key.key;
}

bool is_out_to_acc(const account_keys_t& acc, const key_output_t& out_key, const public_key_t& tx_pub_key, size_t keyIndex) {
  key_derivation_t derivation;
  generate_key_derivation((const uint8_t*)&tx_pub_key, (const uint8_t*)&acc.viewSecretKey, (uint8_t*)&derivation);
  return is_out_to_acc(acc, out_key, derivation, keyIndex);
}

bool lookup_acc_outs(const account_keys_t& acc, const transaction_t& tx, std::vector<size_t>& outs, uint64_t& money_transfered) {
  public_key_t transactionPublicKey = getTransactionPublicKeyFromExtra(tx.extra);
  if (transactionPublicKey == NULL_PUBLIC_KEY)
    return false;
  return lookup_acc_outs(acc, tx, transactionPublicKey, outs, money_transfered);
}

bool lookup_acc_outs(const account_keys_t& acc, const transaction_t& tx, const public_key_t& tx_pub_key, std::vector<size_t>& outs, uint64_t& money_transfered) {
  money_transfered = 0;
  size_t keyIndex = 0;
  size_t outputIndex = 0;

  key_derivation_t derivation;
  generate_key_derivation((const uint8_t*)&tx_pub_key, (const uint8_t*)&acc.viewSecretKey, (uint8_t*)&derivation);

  for (const transaction_output_t& o : tx.outputs) {
    assert(o.target.type() == typeid(key_output_t) || o.target.type() == typeid(multi_signature_output_t));
    if (o.target.type() == typeid(key_output_t)) {
      if (is_out_to_acc(acc, boost::get<key_output_t>(o.target), derivation, keyIndex)) {
        outs.push_back(outputIndex);
        money_transfered += o.amount;
      }

      ++keyIndex;
    } else if (o.target.type() == typeid(multi_signature_output_t)) {
      keyIndex += boost::get<multi_signature_output_t>(o.target).keys.size();
    }

    ++outputIndex;
  }
  return true;
}

std::vector<uint32_t> relative_output_offsets_to_absolute(const std::vector<uint32_t>& off) {
  std::vector<uint32_t> res = off;
  for (size_t i = 1; i < res.size(); i++)
    res[i] += res[i - 1];
  return res;
}

std::vector<uint32_t> absolute_output_offsets_to_relative(const std::vector<uint32_t>& off) {
  std::vector<uint32_t> res = off;
  if (!off.size())
    return res;
  std::sort(res.begin(), res.end());//just to be sure, actually it is already should be sorted
  for (size_t i = res.size() - 1; i != 0; i--)
    res[i] -= res[i - 1];

  return res;
}

void get_tx_tree_hash(const std::vector<hash_t>& tx_hashes, hash_t& h) {
  tree_hash((const char (*)[HASH_SIZE])tx_hashes.data(), tx_hashes.size(), (char *)&h);
}

hash_t get_tx_tree_hash(const std::vector<hash_t>& tx_hashes) {
  hash_t h = NULL_HASH;
  get_tx_tree_hash(tx_hashes, h);
  return h;
}

hash_t get_tx_tree_hash(const block_t& b) {
  std::vector<hash_t> txs_ids;
  hash_t h = NULL_HASH;
  BinaryArray::objectHash(b.baseTransaction, h);
  txs_ids.push_back(h);
  for (auto& th : b.transactionHashes) {
    txs_ids.push_back(th);
  }
  return get_tx_tree_hash(txs_ids);
}

}
