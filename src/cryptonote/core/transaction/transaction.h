#pragma once

#include "crypto.h"
#include "BlockchainExplorerData.h"
#include "types.h"

namespace cryptonote
{
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