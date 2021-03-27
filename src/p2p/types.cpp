
#include <string>
#include "common/str.h"
#include "types.h"

namespace cryptonote
{

  bool parseSocket(const std::string &socket, network_address_t &pe)
  {
    auto start = 0U;
    auto end = socket.find(":");

    std::string ip = socket.substr(start, end);
    start = end + 1;
    std::string port = socket.substr(start);
    pe.ip = ::string::IPv4::from(ip);
    pe.port = ::string::Port::from(port);

    return pe.ip != UINT32_MAX && pe.port != UINT32_MAX;
  }

}