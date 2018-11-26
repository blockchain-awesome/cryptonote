#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "types.h"
#include "cryptonote.h"

namespace cryptonote
{

//
// ITransactionReader
//
class ITransactionReader
{
  public:
    virtual ~ITransactionReader() {}

    virtual crypto::hash_t getTransactionHash() const = 0;
    virtual crypto::hash_t getTransactionPrefixHash() const = 0;
    virtual crypto::public_key_t getTransactionPublicKey() const = 0;
    virtual bool getTransactionSecretKey(crypto::secret_key_t &key) const = 0;
    virtual uint64_t getUnlockTime() const = 0;

    // extra
    virtual bool getPaymentId(crypto::hash_t &paymentId) const = 0;
    virtual bool getExtraNonce(binary_array_t &nonce) const = 0;
    virtual binary_array_t getExtra() const = 0;

    // inputs
    virtual size_t getInputCount() const = 0;
    virtual uint64_t getInputTotalAmount() const = 0;
    virtual TransactionTypes::input_type_t getInputType(size_t index) const = 0;
    virtual void getInput(size_t index, key_input_t &input) const = 0;
    virtual void getInput(size_t index, multi_signature_input_t &input) const = 0;

    // outputs
    virtual size_t getOutputCount() const = 0;
    virtual uint64_t getOutputTotalAmount() const = 0;
    virtual TransactionTypes::output_type_t getOutputType(size_t index) const = 0;
    virtual void getOutput(size_t index, key_output_t &output, uint64_t &amount) const = 0;
    virtual void getOutput(size_t index, multi_signature_output_t &output, uint64_t &amount) const = 0;

    // signatures
    virtual size_t getRequiredSignaturesCount(size_t inputIndex) const = 0;
    virtual bool findOutputsToAccount(const account_public_address_t &addr, const crypto::secret_key_t &viewSecretKey, std::vector<uint32_t> &outs, uint64_t &outputAmount) const = 0;

    // various checks
    virtual bool validateInputs() const = 0;
    virtual bool validateOutputs() const = 0;
    virtual bool validateSignatures() const = 0;

    // serialized transaction
    virtual binary_array_t getTransactionData() const = 0;
};

} // namespace cryptonote