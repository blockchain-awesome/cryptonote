#pragma once

#include "crypto/chacha.h"
#include "Serialization/ISerializer.h"
#include "crypto/crypto.h"

template <typename T>
bool serializePod(T& v, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializer.binary(&v, sizeof(v), name);
}

namespace crypto
{
// template <typename T>
// bool serialize(T &t, Common::StringView name, cryptonote::ISerializer &serializer)
// {
//     return serializer.binary(&t, sizeof(t), name);
// }

bool serialize(PublicKey &pubKey, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(SecretKey &secKey, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(Hash &h, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(chacha_iv &chacha, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(KeyImage &keyImage, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(Signature &sig, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(EllipticCurveScalar &ecScalar, Common::StringView name, cryptonote::ISerializer &serializer);
bool serialize(EllipticCurvePoint &ecPoint, Common::StringView name, cryptonote::ISerializer &serializer);

} // namespace crypto