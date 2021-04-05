#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "crypto/crypto.h"
#include <boost/variant.hpp>
#include "cryptonote/types.h"

namespace cryptonote
{
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
  };

  struct transaction_entry_t
  {
    transaction_t tx;
    std::vector<uint32_t> m_global_output_indexes;
  };
} // namespace cryptonote