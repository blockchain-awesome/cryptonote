#pragma once
#include "serialization/ISerializer.h"

#include <cstdint>
#include <unordered_map>
#include "crypto/types.h"
#include <array>
#include <string>
#include <vector>
#include "crypto/crypto.h"
#include <boost/variant.hpp>

struct transaction_index_t
{
  uint32_t block;
  uint16_t transaction;
};

typedef std::unordered_map<hash_t, transaction_index_t> transaction_map_t;

namespace cryptonote
{
  extern const hash_t NULL_HASH;
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

  struct multisignature_output_usage_t
  {
    transaction_index_t transactionIndex;
    uint16_t outputIndex;
    bool isUsed;
    void serialize(ISerializer &s);
  };

  struct transaction_entry_t
  {
    transaction_t tx;
    std::vector<uint32_t> m_global_output_indexes;
    void serialize(ISerializer &s);
  };

  struct block_info_t
  {
    uint32_t height;
    hash_t id;
    block_info_t()
    {
      clear();
    }

    void clear()
    {
      height = 0;
      id = NULL_HASH;
    }

    bool empty() const
    {
      return id == NULL_HASH;
    }
  };

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

  struct block_entry_t
  {
    block_t bl;
    uint32_t height;
    uint64_t block_cumulative_size;
    difficulty_t cumulative_difficulty;
    uint64_t already_generated_coins;
    std::vector<transaction_entry_t> transactions;
    void serialize(ISerializer &s);
  };

  struct transaction_check_info_t
  {
    block_info_t maxUsedBlock;
    block_info_t lastFailedBlock;
  };

  struct transaction_details_t : public transaction_check_info_t
  {
    hash_t id;
    transaction_t tx;
    size_t blobSize;
    uint64_t fee;
    bool keptByBlock;
    time_t receiveTime;
  };

  struct account_public_address_t
  {
    public_key_t spendPublicKey;
    public_key_t viewPublicKey;
  };

  struct account_keys_t
  {
    account_public_address_t address;
    secret_key_t spendSecretKey;
    secret_key_t viewSecretKey;
  };

  struct key_pair_t
  {
    public_key_t publicKey;
    secret_key_t secretKey;
  };

  // State types

  struct core_state_info_t
  {
    uint64_t tx_pool_size;
    uint64_t blockchain_height;
    uint64_t mining_speed;
    uint64_t alternative_blocks;
    std::string top_block_id_str;
  };

} // namespace cryptonote