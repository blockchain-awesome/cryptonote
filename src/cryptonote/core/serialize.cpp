#include "serialize.h"

bool parse_hash256(const std::string& str_hash, crypto::Hash& hash) {
  return Common::podFromHex(str_hash, hash);
}

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

bool serialize(signature_t& sig, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializePod(sig, name, serializer);
}

bool serialize(EllipticCurveScalar& ecScalar, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializePod(ecScalar, name, serializer);
}

bool serialize(EllipticCurvePoint& ecPoint, Common::StringView name, cryptonote::ISerializer& serializer) {
  return serializePod(ecPoint, name, serializer);
}

}