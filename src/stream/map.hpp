#include "writer.h"
#include "reader.h"
#include "crypto/types.h"
#include "stream/crypto.h"
#include "stream/cryptonote.h"
#include "stream/block.h"
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace stream
{

  namespace cryptonote
  {

    template <typename T>
    Reader &readMap(Reader &i, T &v)
    {

      size_t size = 0;

      i >> size;

      for (size_t j = 0; j < size; j++)
      {
        typename T::key_type key;
        typename T::mapped_type idx;
        i >> key;
        i >> idx;
        v.insert(std::make_pair(std::move(key), std::move(idx)));
      }
      return i;
    }

    template <typename T>
    Writer &writeMap(Writer &o, const T &v)
    {
      size_t size = v.size();
      o << size;

      for (auto &kv : v)
      {
        o << kv.first;
        o << kv.second;
      }
      return o;
    }

    // template <typename KEY, typename VALUE, typename HASH>
    // Reader &operator>>(Reader &i, std::unordered_map<KEY, VALUE, HASH> &v)
    // {
    //   return readMap(i, v);
    // }

    // template <typename KEY, typename VALUE, typename HASH>
    // Writer &operator<<(Writer &o, const std::unordered_map<KEY, VALUE, HASH> &v)
    // {
    //   return writeMap(o, v);
    // }

    // template <typename KEY, typename VALUE, typename HASH>
    // Reader &operator>>(Reader &i, std::unordered_multimap<KEY, VALUE, HASH> &v)
    // {
    //   return readMap(i, v);
    // }

    // template <typename KEY, typename VALUE>
    // Reader &operator>>(Reader &i, std::unordered_multimap<KEY, VALUE> &v)
    // {
    //   return readMap(i, v);
    // }
    // template <typename KEY, typename VALUE, typename HASH>
    // Reader &operator>>(Reader &i, std::map<KEY, VALUE, HASH> &v);
    // template <typename KEY, typename VALUE, typename HASH>
    // Reader &operator>>(Reader &i, std::multimap<KEY, VALUE, HASH> &v);

    // template <typename KEY, typename VALUE, typename HASH>
    // Writer &operator<<(Writer &o, const std::unordered_multimap<KEY, VALUE, HASH> &v)
    // {
    //   return writeMap(o, v);
    // }

    // template <typename KEY, typename VALUE>
    // Writer &operator<<(Writer &o, const std::unordered_multimap<KEY, VALUE> &v)
    // {
    //   return writeMap(o, v);
    // }
    // template <typename KEY, typename VALUE, typename HASH>
    // Writer &operator>>(Writer &o, const std::map<KEY, VALUE, HASH> &v);
    // template <typename KEY, typename VALUE, typename HASH>
    // Writer &operator>>(Writer &o, const std::multimap<KEY, VALUE, HASH> &v);
    //     template<typename K, typename V, typename Hash>
    // bool serialize(std::unordered_map<K, V, Hash>& value, Common::StringView name, cryptonote::ISerializer& serializer) {
    //   return serializeMap(value, name, serializer, [&value](size_t size) { value.reserve(size); });
    // }

    // template<typename K, typename V, typename Hash>
    // bool serialize(std::unordered_multimap<K, V, Hash>& value, Common::StringView name, cryptonote::ISerializer& serializer) {
    //   return serializeMap(value, name, serializer, [&value](size_t size) { value.reserve(size); });
    // }

    // template<typename K, typename V, typename Hash>
    // bool serialize(std::map<K, V, Hash>& value, Common::StringView name, cryptonote::ISerializer& serializer) {
    //   return serializeMap(value, name, serializer, [](size_t size) {});
    // }

    // template<typename K, typename V, typename Hash>
    // bool serialize(std::multimap<K, V, Hash>& value, Common::StringView name, cryptonote::ISerializer& serializer) {
    //   return serializeMap(value, name, serializer, [](size_t size) {});
    // }
  }

}