#pragma once

#include <vector>
#include <boost/variant.hpp>
#include "crypto.h"
#include "BlockchainExplorerData.h"

namespace cryptonote
{

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

class Transaction
{
  public:
    Transaction(transaction_t &tr) : m_transaction(tr) {}
    transaction_t m_transaction;
    uint64_t getInputAmount();
    uint64_t getOutputAmount();
    uint64_t getInputAmount(const transaction_input_t &in);
    uint64_t getMixin(uint64_t mixin = 0);
    void getInputDetails(std::vector<transaction_input_details_t> inputs);
};
} // namespace cryptonote