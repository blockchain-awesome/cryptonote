#pragma once

#include <unordered_map>
#include "cryptonote/crypto/hash.h"
#include "cryptonote/core/key.h"

#include "stream/cryptonote.h"

using namespace stream::cryptonote;

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
  friend Reader &operator>>(Reader &i, PaymentIdIndex &v);

  friend Writer &operator<<(Writer &o, const PaymentIdIndex &v);
  private:
    std::unordered_multimap<hash_t, hash_t> index;
  };

  Reader &operator>>(Reader &i, PaymentIdIndex &v);

  Writer &operator<<(Writer &o, const PaymentIdIndex &v);

} // namespace cryptonote
