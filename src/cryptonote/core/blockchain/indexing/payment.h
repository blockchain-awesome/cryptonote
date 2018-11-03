#pragma once

#include <unordered_map>
#include "hash.h"
#include "cryptonote/core/key.h"

namespace cryptonote
{

class ISerializer;

class PaymentIdIndex
{
  public:
    PaymentIdIndex() = default;

    bool add(const Transaction &transaction);
    bool remove(const Transaction &transaction);
    bool find(const crypto::hash_t &paymentId, std::vector<crypto::hash_t> &transactionHashes);
    void clear();

    void serialize(ISerializer &s);

    template <class Archive>
    void serialize(Archive &archive, unsigned int version)
    {
        archive &index;
    }

  private:
    std::unordered_multimap<crypto::hash_t, crypto::hash_t> index;
};

} // namespace cryptonote
