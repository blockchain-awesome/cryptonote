#pragma once

#include <unordered_map>
#include "cryptonote/crypto/hash.h"
#include "cryptonote/core/key.h"
#include "stream/map.hpp"

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

    template <class Archive>
    void serialize(Archive &archive, unsigned int version)
    {
      archive &index;
    }

    std::unordered_multimap<hash_t, hash_t> index;
  };

  Reader &operator>>(Reader &i, PaymentIdIndex &v);

  Writer &operator<<(Writer &o, const PaymentIdIndex &v);

} // namespace cryptonote
