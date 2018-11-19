#include "array.h"
#include "../core/CryptoNoteTools.h"
#include "../core/CryptoNoteFormatUtils.h"

namespace cryptonote
{

std::string BinaryArray::toString()
{
  return Common::asString(m_ba);
}

bool toBinaryArray(const binary_array_t &object, binary_array_t &binaryArray)
{
  try
  {
    Common::VectorOutputStream stream(binaryArray);
    BinaryOutputStreamSerializer serializer(stream);
    std::string oldBlob = Common::asString(object);
    serializer(oldBlob, "");
  }
  catch (std::exception &)
  {
    return false;
  }

  return true;
}

crypto::hash_t BinaryArray::getHash()
{
  crypto::hash_t hash;
  cn_fast_hash(m_ba.data(), m_ba.size(), hash);
  return hash;
}

void BinaryArray::getHash(const binary_array_t &binaryArray, crypto::hash_t &hash)
{
  cn_fast_hash(binaryArray.data(), binaryArray.size(), hash);
}

crypto::hash_t BinaryArray::getHash(const binary_array_t &binaryArray)
{
  crypto::hash_t hash;
  getHash(binaryArray, hash);
  return hash;
}
} // namespace cryptonote