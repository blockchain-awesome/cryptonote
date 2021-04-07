#include "map.h"

namespace cryptonote
{

  Reader &operator>>(Reader &i, std::unordered_multimap<hash_t, hash_t> &v)
  {
    return readMap(i, v);
  }

  Writer &operator<<(Writer &o, const std::unordered_multimap<hash_t, hash_t> &v)
  {
    return writeMap(o, v);
  }
  Reader &operator>>(Reader &i, std::multimap<uint64_t, hash_t> &v)
  {
    return readMap(i, v);
  }

  Writer &operator<<(Writer &o, const std::multimap<uint64_t, hash_t> &v)
  {
    return writeMap(o, v);
  }
}