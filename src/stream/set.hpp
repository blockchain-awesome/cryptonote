#pragma once
#include "writer.h"
#include "reader.h"
#include "crypto/types.h"
#include "crypto.h"
#include "cryptonote.h"
#include "block.h"
#include <set>
#include <unordered_set>

template <typename T>
Reader &readSet(Reader &i, T &v)
{

  uint64_t size = 0;

  i >> size;

  for (size_t j = 0; j < size; j++)
  {
    typename T::value_type key;
    i >> key;
    v.insert(std::move(key));
  }
  return i;
}

template <typename T>
Writer &writeSet(Writer &o, const T &v)
{
  uint64_t size = v.size();
  o << size;

  for (auto &v : v)
  {
    o << v;
  }
  return o;
}

template <typename KEY, typename VALUE>
Reader &operator>>(Reader &i, std::pair<KEY, VALUE> &v)
{
  i >> v.first;
  i >> v.second;
  return i;
}

template <typename KEY, typename VALUE>
Writer &operator<<(Writer &o, const std::pair<KEY, VALUE> &v)
{
  o << v.first;
  o << v.second;
  return o;
}

template <typename KEY, typename VALUE, typename HASH>
Reader &operator>>(Reader &i, std::unordered_set<KEY, VALUE, HASH> &v)
{
  return readSet(i, v);
}

template <typename KEY, typename VALUE, typename HASH>
Writer &operator<<(Writer &o, const std::unordered_set<KEY, VALUE, HASH> &v)
{
  return writeSet(o, v);
}

template <typename KEY, typename VALUE, typename HASH>
Reader &operator>>(Reader &i, std::set<KEY, VALUE, HASH> &v)
{
  return readSet(i, v);
}

template <typename KEY, typename VALUE, typename HASH>
Writer &operator<<(Writer &o, const std::set<KEY, VALUE, HASH> &v)
{
  return writeSet(o, v);
}
