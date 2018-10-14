#pragma once

#include <unordered_map>
#include "hash.h"
#include "cryptonote/core/CryptoNoteBasic.h"

namespace cryptonote
{

class ISerializer;

class PaymentIdIndex
{
  public:
    PaymentIdIndex() = default;

    bool add(const Transaction &transaction);
    bool remove(const Transaction &transaction);
    bool find(const crypto::Hash &paymentId, std::vector<crypto::Hash> &transactionHashes);
    void clear();

    void serialize(ISerializer &s);

    template <class Archive>
    void serialize(Archive &archive, unsigned int version)
    {
        archive &index;
    }

  private:
    std::unordered_multimap<crypto::Hash, crypto::Hash> index;
};

} // namespace cryptonote
