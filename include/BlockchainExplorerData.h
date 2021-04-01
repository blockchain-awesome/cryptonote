// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <array>
#include <string>
#include <vector>

#include "crypto.h"

#include <boost/variant.hpp>

using namespace crypto;

namespace cryptonote
{

enum class transaction_remove_reason_t : uint8_t
{
  INCLUDED_IN_BLOCK = 0,
  TIMEOUT = 1
};

struct transaction_output_to_key_details_t
{
  public_key_t txOutKey;
};

struct transaction_output_multi_signature_details_t
{
  std::vector<public_key_t> keys;
  uint32_t requiredSignatures;
};

struct transaction_output_details_t
{
  uint64_t amount;
  uint32_t globalIndex;

  boost::variant<
      transaction_output_to_key_details_t,
      transaction_output_multi_signature_details_t>
      output;
};

struct transaction_output_reference_details_t
{
  hash_t transactionHash;
  size_t number;
};

struct transaction_input_generate_details_t
{
  uint32_t height;
};

struct transaction_input_to_key_details_t
{
  std::vector<uint32_t> outputIndexes;
  key_image_t keyImage;
  uint64_t mixin;
  transaction_output_reference_details_t output;
};

struct transaction_input_multisignature_details_t
{
  uint32_t signatures;
  transaction_output_reference_details_t output;
};

typedef boost::variant<
    transaction_input_generate_details_t,
    transaction_input_to_key_details_t,
    transaction_input_multisignature_details_t>
    transaction_input_details_base_t;

struct transaction_input_details_t
{
  uint64_t amount;
  transaction_input_details_base_t input;
};

struct transaction_extra_details_t
{
  std::vector<size_t> padding;
  std::vector<public_key_t> publicKey;
  std::vector<std::string> nonce;
  std::vector<uint8_t> raw;
};

struct transaction_details_t
{
  hash_t hash;
  uint64_t size;
  uint64_t fee;
  uint64_t totalInputsAmount;
  uint64_t totalOutputsAmount;
  uint64_t mixin;
  uint64_t unlockTime;
  uint64_t timestamp;
  hash_t paymentId;
  bool inBlockchain;
  hash_t blockHash;
  uint32_t blockHeight;
  transaction_extra_details_t extra;
  std::vector<std::vector<signature_t>> signatures;
  std::vector<transaction_input_details_t> inputs;
  std::vector<transaction_output_details_t> outputs;
};

struct block_details_t
{
  uint8_t majorVersion;
  uint8_t minorVersion;
  uint64_t timestamp;
  hash_t prevBlockHash;
  uint32_t nonce;
  bool isOrphaned;
  uint32_t height;
  hash_t hash;
  uint64_t difficulty;
  uint64_t reward;
  uint64_t baseReward;
  uint64_t blockSize;
  uint64_t transactionsCumulativeSize;
  uint64_t alreadyGeneratedCoins;
  uint64_t alreadyGeneratedTransactions;
  uint64_t sizeMedian;
  double penalty;
  uint64_t totalFeeAmount;
  std::vector<transaction_details_t> transactions;
};

} // namespace cryptonote
