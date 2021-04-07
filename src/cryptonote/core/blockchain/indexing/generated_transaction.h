#pragma once

#include <unordered_map>
#include "cryptonote/core/key.h"
#include "stream/map.hpp"

namespace cryptonote
{

  class ISerializer;

  class GeneratedTransactionsIndex
  {
  public:
    GeneratedTransactionsIndex();

    bool add(const block_t &block);
    bool remove(const block_t &block);
    bool find(uint32_t height, uint64_t &generatedTransactions);
    void clear();

    void serialize(ISerializer &s);

    template <class Archive>
    void serialize(Archive &archive, unsigned int version)
    {
      archive &index;
      archive &lastGeneratedTxNumber;
    }

    std::unordered_map<uint32_t, uint64_t> index;
    uint64_t lastGeneratedTxNumber;
  };

  Reader &operator>>(Reader &i, GeneratedTransactionsIndex &v);

  Writer &operator<<(Writer &o, const GeneratedTransactionsIndex &v);
} // namespace cryptonote