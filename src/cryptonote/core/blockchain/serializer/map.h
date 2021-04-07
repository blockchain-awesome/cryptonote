#include "stream/map.hpp"

namespace cryptonote
{
  Reader &operator>>(Reader &i, std::unordered_multimap<hash_t, hash_t> &v);
  Writer &operator<<(Writer &o, const std::unordered_multimap<hash_t, hash_t> &v);

  Reader &operator>>(Reader &i, std::multimap<uint64_t, hash_t> &v);
  Writer &operator<<(Writer &o, const std::multimap<uint64_t, hash_t> &v);
}