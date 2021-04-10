
#include "array.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include "serialization/BinaryInputStreamSerializer.h"
#include "stream/reader.h"
#include "cryptonote/core/transaction/serializer/basics.h"

namespace cryptonote
{
template <class T>
bool BinaryArray::from(T &object, const binary_array_t &binaryArray)
{
  bool result = false;
  try
  {
    const unsigned char * b = static_cast<const unsigned char *>(binaryArray.data());
    membuf mem((char *)(b), (char *)(b + binaryArray.size()));
    std::istream istream(&mem);
    Reader stream(istream);
    stream >> object;
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
    std::ostringstream oss;
    Writer stream(oss);
    stream << const_cast<T &>(object);

    binaryArray.resize(oss.str().length());
    memcpy(&binaryArray[0], oss.str().c_str(), oss.str().length());
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
bool BinaryArray::objectHash(const T &object, hash_t &hash)
{
  binary_array_t ba;
  if (!BinaryArray::to(object, ba))
  {
    hash = NULL_HASH;
    return false;
  }

  hash = BinaryArray::hash(ba);
  return true;
}

template <class T>
bool BinaryArray::objectHash(const T &object, hash_t &hash, size_t &size)
{
  binary_array_t ba;
  if (!BinaryArray::to(object, ba))
  {
    hash = NULL_HASH;
    size = (std::numeric_limits<size_t>::max)();
    return false;
  }

  size = ba.size();
  hash = BinaryArray::hash(ba);
  return true;
}

template <class T>
hash_t BinaryArray::objectHash(const T &object)
{
  hash_t hash;
  BinaryArray::objectHash(object, hash);
  return hash;
}

} // namespace cryptonote