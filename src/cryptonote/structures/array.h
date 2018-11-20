#pragma once

#include <vector>
#include <boost/variant.hpp>
#include "crypto.h"
#include "common/StringTools.h"
#include "stream/MemoryInputStream.h"
#include "stream/VectorOutputStream.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include "serialization/BinaryInputStreamSerializer.h"

namespace cryptonote
{
using binary_array_t = std::vector<uint8_t>;
extern const crypto::hash_t NULL_HASH;
class BinaryArray
{
public:
  BinaryArray(binary_array_t &ba) : m_ba(ba){};
  std::string toString();

  // static functions retrieved from old code
  static void getHash(const binary_array_t &binaryArray, crypto::hash_t &hash);
  static crypto::hash_t getHash(const binary_array_t &binaryArray);

  template <class T>
  static bool from(T &object, const binary_array_t &binaryArray);

  template <class T>
  static bool to(const T &object, binary_array_t &binaryArray);
  template <class T>
  static binary_array_t to(const T &object);

  // Object Hash
  template <class T>
  static bool objectHash(const T &object, crypto::hash_t &hash);
  template <class T>
  static bool objectHash(const T &object, crypto::hash_t &hash, size_t &size);
  template <class T>
  static crypto::hash_t objectHash(const T &object);

  template <class T>
  static bool size(const T &object, size_t &size);
  template <class T>
  static size_t size(const T &object);
  crypto::hash_t getHash();

private:
  binary_array_t m_ba;
};

template <class T>
bool BinaryArray::from(T &object, const binary_array_t &binaryArray)
{
  bool result = false;
  try
  {
    Common::MemoryInputStream stream(binaryArray.data(), binaryArray.size());
    BinaryInputStreamSerializer serializer(stream);
    serialize(object, serializer);
    result = stream.endOfStream(); // check that all data was consumed
  }
  catch (std::exception &)
  {
    return false;
  }
  return result;
}

template <class T>
bool BinaryArray::to(const T &object, binary_array_t &binaryArray)
{
  try
  {
    ::Common::VectorOutputStream stream(binaryArray);
    BinaryOutputStreamSerializer serializer(stream);
    serialize(const_cast<T &>(object), serializer);
  }
  catch (std::exception &)
  {
    return false;
  }

  return true;
};

template <class T>
binary_array_t BinaryArray::to(const T &object)
{
  binary_array_t ba;
  BinaryArray::to(object, ba);
  return ba;
}

template <>
bool BinaryArray::to(const binary_array_t &object, binary_array_t &binaryArray);

template <class T>
bool BinaryArray::size(const T &object, size_t &size)
{
  binary_array_t ba;
  if (!BinaryArray::to(object, ba))
  {
    size = (std::numeric_limits<size_t>::max)();
    return false;
  }

  size = ba.size();
  return true;
}

template <class T>
size_t BinaryArray::size(const T &object)
{
  size_t size;
  BinaryArray::size(object, size);
  return size;
}

template <class T>
bool BinaryArray::objectHash(const T &object, crypto::hash_t &hash)
{
  binary_array_t ba;
  if (!BinaryArray::to(object, ba))
  {
    hash = NULL_HASH;
    return false;
  }

  hash = BinaryArray::getHash(ba);
  return true;
}

template <class T>
bool BinaryArray::objectHash(const T &object, crypto::hash_t &hash, size_t &size)
{
  binary_array_t ba;
  if (!BinaryArray::to(object, ba))
  {
    hash = NULL_HASH;
    size = (std::numeric_limits<size_t>::max)();
    return false;
  }

  size = ba.size();
  hash = BinaryArray::getHash(ba);
  return true;
}

template <class T>
crypto::hash_t BinaryArray::objectHash(const T &object)
{
  crypto::hash_t hash;
  BinaryArray::objectHash(object, hash);
  return hash;
}

} // namespace cryptonote