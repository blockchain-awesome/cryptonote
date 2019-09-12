// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
// ITransactionWriter
//
class ITransactionWriter
{
  public:
    virtual ~ITransactionWriter() {}

    // transaction parameters
    virtual void setUnlockTime(uint64_t unlockTime) = 0;

    // extra
    virtual void setPaymentId(const hash_t &paymentId) = 0;
    virtual void setExtraNonce(const binary_array_t &nonce) = 0;
    virtual void appendExtra(const binary_array_t &extraData) = 0;

    // Inputs/Outputs
    virtual size_t addInput(const key_input_t &input) = 0;
    virtual size_t addInput(const multi_signature_input_t &input) = 0;
    virtual size_t addInput(const account_keys_t &senderKeys, const TransactionTypes::input_key_info_t &info, key_pair_t &ephKeys) = 0;

    virtual size_t addOutput(uint64_t amount, const account_public_address_t &to) = 0;
    virtual size_t addOutput(uint64_t amount, const std::vector<account_public_address_t> &to, uint32_t requiredSignatures) = 0;
    virtual size_t addOutput(uint64_t amount, const key_output_t &out) = 0;
    virtual size_t addOutput(uint64_t amount, const multi_signature_output_t &out) = 0;

    // transaction info
    virtual void setTransactionSecretKey(const secret_key_t &key) = 0;

    // signing
    virtual void signInputKey(size_t input, const TransactionTypes::input_key_info_t &info, const key_pair_t &ephKeys) = 0;
    virtual void signInputMultisignature(size_t input, const public_key_t &sourceTransactionKey, size_t outputIndex, const account_keys_t &accountKeys) = 0;
    virtual void signInputMultisignature(size_t input, const key_pair_t &ephemeralKeys) = 0;
};

} // namespace cryptonote
