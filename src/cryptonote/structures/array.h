#pragma once

#include <vector>
#include <boost/variant.hpp>
#include "crypto.h"

namespace cryptonote
{
using binary_array_t = std::vector<uint8_t>;

class BinaryArray
{
public:
  BinaryArray(binary_array_t &ba) : m_ba(ba){};
  std::string toString();

  // static functions retrieved from old code
  static void getHash(const binary_array_t &binaryArray, crypto::hash_t &hash);
  static crypto::hash_t getHash(const binary_array_t &binaryArray);

  static bool to(const binary_array_t &object, binary_array_t &binaryArray);
  // template <class T>
  // static bool to(const T &object, binary_array_t &binaryArray);
  // template <class T>
  // static binary_array_t to(const T &object);
  crypto::hash_t getHash();

private:
  binary_array_t m_ba;
};

// template <class T>
// bool toBinaryArray(const T &object, binary_array_t &binaryArray)
// {
//   try
//   {
//     Common::VectorOutputStream stream(binaryArray);
//     BinaryOutputStreamSerializer serializer(stream);
//     std::string oldBlob = Common::asString(object);
//     serializer(oldBlob, "");
//   }
//   catch (std::exception &)
//   {
//     return false;
//   }

//   return true;
// };

// template <class T>
// binary_array_t toBinaryArray(const T &object)
// {
//   binary_array_t ba;
//   toBinaryArray(object, ba);
//   return ba;
// }

} // namespace cryptonote