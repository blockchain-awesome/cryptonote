#include "array.hpp"
#include "crypto/hash-ops.h"


using namespace crypto;

namespace cryptonote
{

std::string BinaryArray::toString()
{
  return IBinary::to(m_ba);
}

template <>
bool BinaryArray::to(const binary_array_t &object, binary_array_t &binaryArray)
{
  try
  {
    std::ostringstream oss;
    Writer stream(oss);
    std::string oldBlob = IBinary::to(object);
    stream << oldBlob;
    binaryArray.resize(oss.str().length());
    memcpy(&binaryArray[0], oss.str().c_str(), oss.str().length());
  }
  catch (std::exception &)
  {
    return false;
  }
  return true;
}

hash_t BinaryArray::hash()
{
  hash_t hash;
  cn_fast_hash(m_ba.data(), m_ba.size(), (char*)&hash);
  return hash;
}


hash_t BinaryArray::hash(const binary_array_t &ba)
{
  hash_t hash;
  cn_fast_hash(ba.data(), ba.size(), (char*)&hash);
  return hash;
}

} // namespace cryptonote