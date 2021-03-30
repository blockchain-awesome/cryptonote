/**
    Copyright (C) 2021-  The VIGCOIN Developer(s)

    @author calidion
    @email calidion@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <cstdint>
#include <unordered_map>
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <boost/variant.hpp>
#include "crypto/types.h"
#include "crypto.h"

#include "google/sparse_hash_set"
#include "google/sparse_hash_map"

enum class input_type_t : uint8_t
{
    Invalid,
    Key,
    Multisignature,
    Generating
};
enum class output_type_t : uint8_t
{
    Invalid,
    Key,
    Multisignature
};

struct global_output_t
{
    public_key_t targetKey;
    uint32_t outputIndex;
};

typedef std::vector<global_output_t> global_output_container_t;

struct output_key_info_t
{
    public_key_t transactionPublicKey;
    size_t transactionIndex;
    size_t outputInTransaction;
};

struct input_key_info_t
{
    uint64_t amount;
    global_output_container_t outputs;
    output_key_info_t realOutput;
};

struct base_input_t
{
    uint32_t blockIndex;
};

struct key_input_t
{
    uint64_t amount;
    std::vector<uint32_t> outputIndexes;
    key_image_t keyImage;
};

struct multi_signature_input_t
{
    uint64_t amount;
    uint8_t signatureCount;
    uint32_t outputIndex;
};

struct key_output_t
{
    public_key_t key;
};

struct multi_signature_output_t
{
    std::vector<public_key_t> keys;
    uint8_t requiredSignatureCount;
};

typedef boost::variant<base_input_t, key_input_t, multi_signature_input_t> transaction_input_t;

typedef boost::variant<key_output_t, multi_signature_output_t> transaction_output_target_t;

struct transaction_output_t
{
    uint64_t amount;
    transaction_output_target_t target;
};

struct transaction_prefix_t
{
    uint8_t version;
    uint64_t unlockTime;
    std::vector<transaction_input_t> inputs;
    std::vector<transaction_output_t> outputs;
    std::vector<uint8_t> extra;
};

struct transaction_t : public transaction_prefix_t
{
    std::vector<std::vector<signature_t>> signatures;
};

struct transaction_index_t
{
    uint32_t block;
    uint16_t transaction;
};

struct transaction_entry_t
{
    transaction_t tx;
    std::vector<uint32_t> m_global_output_indexes;
};

typedef uint64_t difficulty_t;

struct block_header_t
{
    uint8_t majorVersion;
    uint8_t minorVersion;
    uint32_t nonce;
    uint64_t timestamp;
    hash_t previousBlockHash;
};

struct block_t : public block_header_t
{
    transaction_t baseTransaction;
    std::vector<hash_t> transactionHashes;
};

struct block_info_t
{
    uint32_t height;
    hash_t id;
};
struct block_entry_t
{
    block_t bl;
    uint32_t height;
    uint64_t block_cumulative_size;
    difficulty_t cumulative_difficulty;
    uint64_t already_generated_coins;
    std::vector<transaction_entry_t> transactions;
};

struct multisignature_output_usage_t
{
    transaction_index_t transactionIndex;
    uint16_t outputIndex;
    bool isUsed;

    // void serialize(ISerializer &s)
    // {
    //     s(transactionIndex, "txindex");
    //     s(outputIndex, "outindex");
    //     s(isUsed, "used");
    // }
};

// complex types

typedef std::unordered_map<hash_t, transaction_index_t> transaction_map_t;
typedef google::sparse_hash_set<key_image_t> key_images_container_t;
typedef std::unordered_map<hash_t, block_entry_t> blocks_ext_by_hash_t;
typedef google::sparse_hash_map<uint64_t, std::vector<std::pair<transaction_index_t, uint16_t>>> outputs_container_t; //hash_t - tx hash, size_t - index of out in transaction
typedef google::sparse_hash_map<uint64_t, std::vector<multisignature_output_usage_t>> multisignature_outputs_container_t;
