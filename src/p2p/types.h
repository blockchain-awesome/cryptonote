#pragma once

#include <string>
#include <boost/uuid/uuid.hpp>

namespace cryptonote
{
  typedef boost::uuids::uuid uuid;
  typedef boost::uuids::uuid net_connection_id;
  typedef uint64_t peer_id_type_t;

#pragma pack(push, 1)

  struct network_address_t
  {
    uint32_t ip;
    uint32_t port;
  };

  struct peerlist_entry_t
  {
    network_address_t adr;
    peer_id_type_t id;
    uint64_t last_seen;
  };

  struct connection_entry_t
  {
    network_address_t adr;
    peer_id_type_t id;
    bool is_income;
  };

#pragma pack(pop)

  bool parseSocket(const std::string &socket, network_address_t &pe);

}
