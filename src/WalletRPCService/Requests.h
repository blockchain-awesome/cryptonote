#include <exception>
#include <limits>
#include <vector>

#include "Serialization/ISerializer.h"

namespace MultiWalletService
{
class RequestSerializationError : public std::exception
{
public:
  virtual const char *what() const throw() override { return "Request error"; }
};

struct Login
{
  struct Request
  {
    std::string spendSecretKey;
    std::string spendPublicKey;
    std::string address;

    void serialize(CryptoNote::ISerializer &serializer);
  };

  struct Response
  {

    void serialize(CryptoNote::ISerializer &serializer);
  };
};

} // namespace MultiWalletService