#include "serialize.h"

namespace crypto {

bool serialize(PublicKey& pubKey, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializePod(pubKey, name, serializer);
}

bool serialize(SecretKey& secKey, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializePod(secKey, name, serializer);
}

bool serialize(Hash& h, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializePod(h, name, serializer);
}

bool serialize(KeyImage& keyImage, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializePod(keyImage, name, serializer);
}

bool serialize(chacha_iv& chacha, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializePod(chacha, name, serializer);
}

bool serialize(Signature& sig, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializePod(sig, name, serializer);
}

bool serialize(EllipticCurveScalar& ecScalar, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializePod(ecScalar, name, serializer);
}

bool serialize(EllipticCurvePoint& ecPoint, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializePod(ecPoint, name, serializer);
}

}