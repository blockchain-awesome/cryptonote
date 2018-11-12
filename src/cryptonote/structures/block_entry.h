#pragma once

#include "block.h"
#include "cryptonote/core/blockchain/serializer/transaction_entry.hpp"

namespace cryptonote
{
struct block_entry_t
{
    block_t bl;
    uint32_t height;
    uint64_t block_cumulative_size;
    difficulty_t cumulative_difficulty;
    uint64_t already_generated_coins;
    std::vector<transaction_entry_t> transactions;

    void serialize(ISerializer &s)
    {
        s(bl, "block");
        s(height, "height");
        s(block_cumulative_size, "block_cumulative_size");
        s(cumulative_difficulty, "cumulative_difficulty");
        s(already_generated_coins, "already_generated_coins");
        s(transactions, "transactions");
    }
};

class Block
{
  public:
    Block(block_entry_t &block) : m_block(block){};

    static bool checkProofOfWork(const block_t &block, difficulty_t currentDiffic, crypto::hash_t &proofOfWork);

    static block_t genesis(config::config_t &conf);

    static std::string toString(const block_entry_t &block);

    std::string toString();

  private:
    block_entry_t m_block;
};
} // namespace cryptonote