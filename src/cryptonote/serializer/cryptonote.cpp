
#include "cryptonote.h"
#include "stream/primitive.h"
#include "stream/crypto.h"

using namespace serialize::primitive;
using namespace serialize::crypto;

namespace serialize
{
  namespace cryptonote
  {
    std::istream &operator>>(std::istream &i, transaction_index_t &v)
    {
      i >> v.block;
      i >> v.transaction;
      return i;
    }

    std::istream &operator>>(std::istream &i, transaction_map_t &v)
    {

      size_t size = 0;
      v.clear();

      i >> size;

      for (size_t j = 0; j < size; j++)
      {
        hash_t key;
        transaction_index_t idx;
        i >> key;
        i >> idx;
        v.insert(std::make_pair(key, idx));
      }
    }

    std::ostream &operator<<(std::ostream &o, const transaction_index_t &v)
    {
      o << v.block;
      o << v.transaction;
      return o;
    }

    std::ostream &operator<<(std::ostream &o, const transaction_map_t &v)
    {

      size_t size = v.size();
      o << size;

      for (auto&kv : v)
      {
        o << kv.first;
        o << kv.second;
      }
    }
  }
}