#pragma once
#include "writer.h"
#include "reader.h"
#include "crypto/types.h"
#include "stream/crypto.h"
#include "stream/cryptonote.h"
#include "stream/block.h"
#include <map>
#include <unordered_map>
#include <unordered_set>

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

template <typename KEY, typename VALUE, typename HASH>
Reader &operator>>(Reader &i, std::unordered_map<KEY, VALUE, HASH> &v)
{
  return readMap(i, v);
}

template <typename KEY, typename VALUE, typename HASH>
Writer &operator<<(Writer &o, const std::unordered_map<KEY, VALUE, HASH> &v)
{
  return writeMap(o, v);
}

template <typename KEY, typename VALUE, typename HASH>
Reader &operator>>(Reader &i, std::unordered_multimap<KEY, VALUE, HASH> &v)
{
  return readMap(i, v);
}

template <typename KEY, typename VALUE, typename HASH>
Writer &operator<<(Writer &o, const std::unordered_multimap<KEY, VALUE, HASH> &v)
{
  return writeMap(o, v);
}

template <typename KEY, typename VALUE, typename HASH>
Reader &operator>>(Reader &i, std::multimap<KEY, VALUE, HASH> &v)
{
  return readMap(i, v);
}

template <typename KEY, typename VALUE, typename HASH>
Writer &operator<<(Writer &o, const std::multimap<KEY, VALUE, HASH> &v)
{
  return writeMap(o, v);
}

template <typename KEY, typename VALUE, typename HASH>
Reader &operator>>(Reader &i, std::map<KEY, VALUE, HASH> &v)
{
  return readMap(i, v);
}

template <typename KEY, typename VALUE, typename HASH>
Writer &operator<<(Writer &o, const std::map<KEY, VALUE, HASH> &v)
{
  return writeMap(o, v);
}

template <typename Element, typename Indexed>
Reader &iterate(Reader &i, Indexed &idx)
{
  size_t size = 0;
  i >> size;
  while (size--)
  {
    Element e;
    i >> e;
    idx.insert(std::move(e));
  }
  return i;
}

template <typename Element, typename Indexed>
Writer &iterate(Writer &o, Indexed &idx)
{
  auto &begin = idx.begin();
  auto &end = idx.end();
  size_t size = std::distance(begin, end);
  o << size;
  for (auto& i = begin; i != end; ++i)
  {
    o << const_cast<Element &>(*i);
  }
  return o;
}
