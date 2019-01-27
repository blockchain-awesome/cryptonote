// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote/core/key.h"
#include "crypto/chacha.h"
#include "serialization/ISerializer.h"
#include "crypto/crypto.h"
#include "crypto.h"

namespace cryptonote {

struct account_keys_t;

void serialize(transaction_prefix_t& txP, ISerializer& serializer);
void serialize(transaction_t& tx, ISerializer& serializer);
void serialize(transaction_input_t& in, ISerializer& serializer);
void serialize(transaction_output_t& in, ISerializer& serializer);

void serialize(base_input_t& gen, ISerializer& serializer);
void serialize(key_input_t& key, ISerializer& serializer);
void serialize(multi_signature_input_t& multisignature, ISerializer& serializer);

void serialize(transaction_output_t& output, ISerializer& serializer);
void serialize(transaction_output_target_t& output, ISerializer& serializer);
void serialize(key_output_t& key, ISerializer& serializer);
void serialize(multi_signature_output_t& multisignature, ISerializer& serializer);

void serialize(block_header_t& header, ISerializer& serializer);
void serialize(block_t& block, ISerializer& serializer);

void serialize(account_public_address_t& address, ISerializer& serializer);
void serialize(account_keys_t& keys, ISerializer& s);

void serialize(key_pair_t& keyPair, ISerializer& serializer);

// For block explorer

void serialize(transaction_output_reference_details_t& tord, ISerializer& serializer);
void serialize(transaction_input_generate_details_t& tigd, ISerializer& serializer);
void serialize(transaction_input_to_key_details_t& titkd, ISerializer& serializer);
void serialize(transaction_input_multisignature_details_t& timd, ISerializer& serializer);
void serialize(transaction_input_details_t& tid, ISerializer& serializer);

}
