// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "cryptonote_core/CryptoNoteBasic.h"
#include "ITransaction.h"

namespace CryptoNote {

bool checkInputsKeyimagesDiff(const CryptoNote::TransactionPrefix& tx);

// TransactionInput helper functions
size_t getRequiredSignaturesCount(const TransactionInput& in);
uint64_t getTransactionInputAmount(const TransactionInput& in);
TransactionTypes::InputType getTransactionInputType(const TransactionInput& in);
const TransactionInput& getInputChecked(const CryptoNote::TransactionPrefix& transaction, size_t index);
const TransactionInput& getInputChecked(const CryptoNote::TransactionPrefix& transaction, size_t index, TransactionTypes::InputType type);

bool isOutToKey(const crypto::PublicKey& spendPublicKey, const crypto::PublicKey& outKey, const crypto::KeyDerivation& derivation, size_t keyIndex);

// TransactionOutput helper functions
TransactionTypes::OutputType getTransactionOutputType(const TransactionOutputTarget& out);
const TransactionOutput& getOutputChecked(const CryptoNote::TransactionPrefix& transaction, size_t index);
const TransactionOutput& getOutputChecked(const CryptoNote::TransactionPrefix& transaction, size_t index, TransactionTypes::OutputType type);

bool findOutputsToAccount(const CryptoNote::TransactionPrefix& transaction, const AccountPublicAddress& addr,
        const crypto::SecretKey& viewSecretKey, std::vector<uint32_t>& out, uint64_t& amount);

} //namespace CryptoNote
