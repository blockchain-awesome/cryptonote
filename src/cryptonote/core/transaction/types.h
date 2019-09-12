#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "crypto.h"


using namespace crypto;

namespace cryptonote
{

namespace TransactionTypes
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
} // namespace TransactionTypes
} // namespace cryptonote