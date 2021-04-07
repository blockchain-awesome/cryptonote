#pragma once

#include "cryptonote/types.h"
#include "config/common.h"
#include "array.h"
#include "crypto/difficulty.h"
#include "cryptonote/core/key.h"
#include "cryptonote/core/transaction/serializer/basics.h"

namespace cryptonote
{

  class Block
  {
  public:
    Block(block_entry_t &block) : m_block(block){};

    //Static tool functions
    static bool getBlob(const block_t &b, binary_array_t &ba);
    static bool getHash(const block_t &block, hash_t &hash);
    static hash_t getHash(const block_t &block);

    static bool getLongHash(const block_t &b, hash_t &res);
    static bool checkProofOfWork(const block_t &block, difficulty_t currentDiffic, hash_t &proofOfWork);

    static block_t genesis(config::config_t &conf);

    static std::string toString(const block_entry_t &block);

    std::string toString();

  private:
    block_entry_t m_block;
  };
} // namespace cryptonote