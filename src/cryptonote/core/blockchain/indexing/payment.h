#pragma once

#include <unordered_map>
#include "cryptonote/hash.h"
#include "cryptonote/core/key.h"

namespace cryptonote
{

class ISerializer;

class PaymentIdIndex
{
  public:
    PaymentIdIndex() = default;

    bool add(const transaction_t &transaction);
    bool remove(const transaction_t &transaction);
    bool find(const hash_t &paymentId, std::vector<hash_t> &transactionHashes);
    void clear();

    void serialize(ISerializer &s);

    template <class Archive>
    void serialize(Archive &archive, unsigned int version)
    {
        archive &index;
    }

  private:
    std::unordered_multimap<hash_t, crypto::hash_t> index;
};

} // namespace cryptonote
