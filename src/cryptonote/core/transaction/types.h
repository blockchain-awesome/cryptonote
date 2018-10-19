#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "crypto.h"

namespace cryptonote
{

namespace TransactionTypes
{

enum class InputType : uint8_t
{
    Invalid,
    Key,
    Multisignature,
    Generating
};
enum class OutputType : uint8_t
{
    Invalid,
    Key,
    Multisignature
};

struct GlobalOutput
{
    crypto::PublicKey targetKey;
    uint32_t outputIndex;
};

typedef std::vector<GlobalOutput> GlobalOutputsContainer;

struct OutputKeyInfo
{
    crypto::PublicKey transactionPublicKey;
    size_t transactionIndex;
    size_t outputInTransaction;
};

struct InputKeyInfo
{
    uint64_t amount;
    GlobalOutputsContainer outputs;
    OutputKeyInfo realOutput;
};
} // namespace TransactionTypes
} // namespace cryptonote