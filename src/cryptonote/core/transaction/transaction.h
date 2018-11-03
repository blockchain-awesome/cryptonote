#pragma once

#include <vector>
#include <boost/variant.hpp>
#include "crypto.h"

namespace cryptonote
{

struct BaseInput
{
    uint32_t blockIndex;
};

struct KeyInput
{
    uint64_t amount;
    std::vector<uint32_t> outputIndexes;
    crypto::KeyImage keyImage;
};

struct MultisignatureInput
{
    uint64_t amount;
    uint8_t signatureCount;
    uint32_t outputIndex;
};

struct KeyOutput
{
    crypto::PublicKey key;
};

struct MultisignatureOutput
{
    std::vector<crypto::PublicKey> keys;
    uint8_t requiredSignatureCount;
};

typedef boost::variant<BaseInput, KeyInput, MultisignatureInput> TransactionInput;

typedef boost::variant<KeyOutput, MultisignatureOutput> TransactionOutputTarget;

struct TransactionOutput
{
    uint64_t amount;
    TransactionOutputTarget target;
};

struct TransactionPrefix
{
    uint8_t version;
    uint64_t unlockTime;
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    std::vector<uint8_t> extra;
};

struct Transaction : public TransactionPrefix
{
    std::vector<std::vector<crypto::signature_t>> signatures;
};

} // namespace cryptonote