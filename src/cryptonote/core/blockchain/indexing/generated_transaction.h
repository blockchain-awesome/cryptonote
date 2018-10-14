#pragma once

#include <unordered_map>
#include "cryptonote/core/CryptoNoteBasic.h"

namespace cryptonote
{

class ISerializer;

class GeneratedTransactionsIndex
{
  public:
    GeneratedTransactionsIndex();

    bool add(const Block &block);
    bool remove(const Block &block);
    bool find(uint32_t height, uint64_t &generatedTransactions);
    void clear();

    void serialize(ISerializer &s);

    template <class Archive>
    void serialize(Archive &archive, unsigned int version)
    {
        archive &index;
        archive &lastGeneratedTxNumber;
    }

  private:
    std::unordered_map<uint32_t, uint64_t> index;
    uint64_t lastGeneratedTxNumber;
};
} // namespace cryptonote