#pragma once

#include "crypto/chacha.h"
#include "serialization/ISerializer.h"
#include "crypto/crypto.h"
#include "common/StringTools.h"

template <typename T>
bool serializePod(T &v, Common::StringView name, cryptonote::ISerializer &serializer)
{
  return serializer.binary(&v, sizeof(v), name);
}

template <class T>
std::ostream &print256(std::ostream &o, const T &v)
{
  return o << "<" << hex::podToString(v) << ">";
}

bool parse_hash256(const std::string &str_hash, crypto::hash_t &hash);

namespace crypto
{
  // Serialize for crypto data
bool serialize(public_key_t &pubKey, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(secret_key_t &secKey, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(hash_t &h, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(chacha_iv_t &chacha, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(key_image_t &keyImage, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(signature_t &sig, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(elliptic_curve_scalar_t &ecScalar, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(elliptic_curve_point_t &ecPoint, Common::StringView name, cryptonote::ISerializer &serializer);

inline std::ostream &operator<<(std::ostream &o, const crypto::public_key_t &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const crypto::secret_key_t &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const crypto::key_derivation_t &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const crypto::key_image_t &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const crypto::signature_t &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const crypto::hash_t &v) { return print256(o, v); }
} // namespace crypto