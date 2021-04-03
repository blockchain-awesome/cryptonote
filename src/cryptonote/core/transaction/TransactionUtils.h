// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "cryptonote/core/key.h"
#include "ITransaction.h"

namespace cryptonote {

bool checkInputsKeyimagesDiff(const cryptonote::transaction_prefix_t& tx);

// transaction_input_t helper functions
size_t getRequiredSignaturesCount(const transaction_input_t& in);
uint64_t getTransactionInputAmount(const transaction_input_t& in);
input_type_t getTransactionInputType(const transaction_input_t& in);
const transaction_input_t& getInputChecked(const cryptonote::transaction_prefix_t& transaction, size_t index);
const transaction_input_t& getInputChecked(const cryptonote::transaction_prefix_t& transaction, size_t index, input_type_t type);

bool isOutToKey(const public_key_t& spendPublicKey, const public_key_t& outKey, const key_derivation_t& derivation, size_t keyIndex);

// transaction_output_t helper functions
output_type_t getTransactionOutputType(const transaction_output_target_t& out);
const transaction_output_t& getOutputChecked(const cryptonote::transaction_prefix_t& transaction, size_t index);
const transaction_output_t& getOutputChecked(const cryptonote::transaction_prefix_t& transaction, size_t index, output_type_t type);

bool findOutputsToAccount(const cryptonote::transaction_prefix_t& transaction, const account_public_address_t& addr,
        const secret_key_t& viewSecretKey, std::vector<uint32_t>& out, uint64_t& amount);

} //namespace cryptonote
