#include "array.hpp"

namespace cryptonote
{

std::string BinaryArray::toString()
{
  return BinaryArray::toString(m_ba);
}

std::string BinaryArray::toString(const void* data, size_t size) {
  return std::string(static_cast<const char*>(data), size);
}

std::string BinaryArray::toString(const binary_array_t& data) {
  return std::string(reinterpret_cast<const char*>(data.data()), data.size());
}

template <>
bool BinaryArray::to(const binary_array_t &object, binary_array_t &binaryArray)
{
  try
  {
    Common::VectorOutputStream stream(binaryArray);
    BinaryOutputStreamSerializer serializer(stream);
    std::string oldBlob = BinaryArray::toString(object);
    serializer(oldBlob, "");
  }
  catch (std::exception &)
  {
    return false;
  }
  return true;
}

hash_t BinaryArray::getHash()
{
  hash_t hash;
  cn_fast_hash(m_ba.data(), m_ba.size(), (char*)&hash);
  return hash;
}

void BinaryArray::getHash(const binary_array_t &binaryArray, hash_t &hash)
{
  cn_fast_hash(binaryArray.data(), binaryArray.size(), (char*)&hash);
}

hash_t BinaryArray::getHash(const binary_array_t &binaryArray)
{
  hash_t hash;
  getHash(binaryArray, hash);
  return hash;
}

} // namespace cryptonote