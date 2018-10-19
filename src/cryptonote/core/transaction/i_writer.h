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
    virtual void setPaymentId(const crypto::Hash &paymentId) = 0;
    virtual void setExtraNonce(const BinaryArray &nonce) = 0;
    virtual void appendExtra(const BinaryArray &extraData) = 0;

    // Inputs/Outputs
    virtual size_t addInput(const KeyInput &input) = 0;
    virtual size_t addInput(const MultisignatureInput &input) = 0;
    virtual size_t addInput(const AccountKeys &senderKeys, const TransactionTypes::InputKeyInfo &info, KeyPair &ephKeys) = 0;

    virtual size_t addOutput(uint64_t amount, const AccountPublicAddress &to) = 0;
    virtual size_t addOutput(uint64_t amount, const std::vector<AccountPublicAddress> &to, uint32_t requiredSignatures) = 0;
    virtual size_t addOutput(uint64_t amount, const KeyOutput &out) = 0;
    virtual size_t addOutput(uint64_t amount, const MultisignatureOutput &out) = 0;

    // transaction info
    virtual void setTransactionSecretKey(const crypto::SecretKey &key) = 0;

    // signing
    virtual void signInputKey(size_t input, const TransactionTypes::InputKeyInfo &info, const KeyPair &ephKeys) = 0;
    virtual void signInputMultisignature(size_t input, const crypto::PublicKey &sourceTransactionKey, size_t outputIndex, const AccountKeys &accountKeys) = 0;
    virtual void signInputMultisignature(size_t input, const KeyPair &ephemeralKeys) = 0;
};

} // namespace cryptonote
