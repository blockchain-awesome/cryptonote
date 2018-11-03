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

    virtual crypto::Hash getTransactionHash() const = 0;
    virtual crypto::Hash getTransactionPrefixHash() const = 0;
    virtual crypto::public_key_t getTransactionPublicKey() const = 0;
    virtual bool getTransactionSecretKey(crypto::secret_key_t &key) const = 0;
    virtual uint64_t getUnlockTime() const = 0;

    // extra
    virtual bool getPaymentId(crypto::Hash &paymentId) const = 0;
    virtual bool getExtraNonce(BinaryArray &nonce) const = 0;
    virtual BinaryArray getExtra() const = 0;

    // inputs
    virtual size_t getInputCount() const = 0;
    virtual uint64_t getInputTotalAmount() const = 0;
    virtual TransactionTypes::InputType getInputType(size_t index) const = 0;
    virtual void getInput(size_t index, KeyInput &input) const = 0;
    virtual void getInput(size_t index, MultisignatureInput &input) const = 0;

    // outputs
    virtual size_t getOutputCount() const = 0;
    virtual uint64_t getOutputTotalAmount() const = 0;
    virtual TransactionTypes::OutputType getOutputType(size_t index) const = 0;
    virtual void getOutput(size_t index, KeyOutput &output, uint64_t &amount) const = 0;
    virtual void getOutput(size_t index, MultisignatureOutput &output, uint64_t &amount) const = 0;

    // signatures
    virtual size_t getRequiredSignaturesCount(size_t inputIndex) const = 0;
    virtual bool findOutputsToAccount(const AccountPublicAddress &addr, const crypto::secret_key_t &viewSecretKey, std::vector<uint32_t> &outs, uint64_t &outputAmount) const = 0;

    // various checks
    virtual bool validateInputs() const = 0;
    virtual bool validateOutputs() const = 0;
    virtual bool validateSignatures() const = 0;

    // serialized transaction
    virtual BinaryArray getTransactionData() const = 0;
};

} // namespace cryptonote