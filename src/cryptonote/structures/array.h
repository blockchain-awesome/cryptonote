#pragma once

#include <vector>
#include <boost/variant.hpp>
#include "common/binary.h"
#include "crypto.h"

using namespace crypto;

namespace cryptonote
{
extern const hash_t NULL_HASH;
class BinaryArray
{
public:
  BinaryArray(binary_array_t &ba) : m_ba(ba){};

  std::string toString();

  static hash_t hash(const binary_array_t &binaryArray);

  template <class T>
  static bool from(T &object, const binary_array_t &binaryArray);

  template <class T>
  static bool to(const T &object, binary_array_t &binaryArray);
  template <class T>
  static binary_array_t to(const T &object);

  // Object Hash
  template <class T>
  static bool objectHash(const T &object, hash_t &hash);
  template <class T>
  static bool objectHash(const T &object, hash_t &hash, size_t &size);
  template <class T>
  static hash_t objectHash(const T &object);

  template <class T>
  static bool size(const T &object, size_t &size);
  template <class T>
  static size_t size(const T &object);
  hash_t hash();

private:
  binary_array_t m_ba;
};

} // namespace cryptonote