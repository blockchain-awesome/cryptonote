
#include "array.h"
#include "stream/VectorOutputStream.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include "serialization/BinaryInputStreamSerializer.h"
#include "stream/MemoryInputStream.h"
#include "cryptonote/core/blockchain/serializer/basics.h"

namespace cryptonote
{
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