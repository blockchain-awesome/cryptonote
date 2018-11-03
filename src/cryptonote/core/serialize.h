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
  return o << "<" << Common::podToHex(v) << ">";
}

bool parse_hash256(const std::string &str_hash, crypto::Hash &hash);

namespace crypto
{
  // Serialize for crypto data
bool serialize(PublicKey &pubKey, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(SecretKey &secKey, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(Hash &h, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(chacha_iv &chacha, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(KeyImage &keyImage, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(signature_t &sig, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(EllipticCurveScalar &ecScalar, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(EllipticCurvePoint &ecPoint, Common::StringView name, cryptonote::ISerializer &serializer);

inline std::ostream &operator<<(std::ostream &o, const crypto::PublicKey &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const crypto::SecretKey &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const crypto::KeyDerivation &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const crypto::KeyImage &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const crypto::signature_t &v) { return print256(o, v); }
inline std::ostream &operator<<(std::ostream &o, const crypto::Hash &v) { return print256(o, v); }
} // namespace crypto