
#include "Requests.h"
// #include "Serialization/SerializationOverloads.h"

namespace MultiWalletService
{

void Login::Request::serialize(CryptoNote::ISerializer &serializer)
{
  bool hasSecretKey = serializer(spendSecretKey, "spendSecretKey");
  bool hasPublicKey = serializer(spendPublicKey, "spendPublicKey");
  bool hasAddress = serializer(address, "address");

  if (hasSecretKey && hasPublicKey && hasAddress)
  {
    //TODO: replace it with error codes
    throw RequestSerializationError();
  }
}

void Login::Response::serialize(CryptoNote::ISerializer &serializer)
{
}
} // namespace MultiWalletService