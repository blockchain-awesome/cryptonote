#pragma once

#include "cryptonote/crypto/chacha.h"
#include "serialization/ISerializer.h"
#include "cryptonote/crypto/crypto.h"
#include "common/hex.h"
#include "common/str.h"

using namespace crypto;

template <typename T>
bool serializePod(T &v, Common::StringView name, cryptonote::ISerializer &serializer)
{
  return serializer.binary(&v, sizeof(v), name);
}

template <class T>
std::ostream &print256(std::ostream &o, const T &v)
{
  return o << "<" << hex::podTo(v) << ">";
}

bool parse_hash256(const std::string &str_hash, hash_t &hash);

  // Serialize for crypto data
bool serialize(public_key_t &pubKey, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(secret_key_t &secKey, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(hash_t &h, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(chacha_iv_t &chacha, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(key_image_t &keyImage, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(signature_t &sig, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(elliptic_curve_scalar_t &ecScalar, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(elliptic_curve_point_t &ecPoint, Common::StringView name, cryptonote::ISerializer &serializer);

inline std::ostream &operator<<(std::ostream &o, const public_key_t &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const secret_key_t &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const key_derivation_t &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const key_image_t &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const signature_t &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const hash_t &v) { return print256(o, v); }
