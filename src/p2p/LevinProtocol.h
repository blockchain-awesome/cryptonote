// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote.h"
#include <stream/memory.h>
#include <stream/VectorOutputStream.h>
#include "serialization/KVBinaryInputStreamSerializer.h"
#include "serialization/KVBinaryOutputStreamSerializer.h"

namespace System {
class TcpConnection;
}

namespace cryptonote {

enum class LevinError: int32_t {
  OK = 0,
  ERROR_CONNECTION = -1,
  ERROR_CONNECTION_NOT_FOUND = -2,
  ERROR_CONNECTION_DESTROYED = -3,
  ERROR_CONNECTION_TIMEDOUT = -4,
  ERROR_CONNECTION_NO_DUPLEX_PROTOCOL = -5,
  ERROR_CONNECTION_HANDLER_NOT_DEFINED = -6,
  ERROR_FORMAT = -7,
};

const int32_t LEVIN_PROTOCOL_RETCODE_SUCCESS = 1;

class LevinProtocol {
public:

  LevinProtocol(System::TcpConnection& connection);

  template <typename Request, typename Response>
  bool invoke(uint32_t command, const Request& request, Response& response) {
    sendMessage(command, encode(request), true);

    Command cmd;
    readCommand(cmd);

    if (!cmd.isResponse) {
      return false;
    }

    return decode(cmd.buf, response); 
  }

  template <typename Request>
  void notify(uint32_t command, const Request& request, int) {
    sendMessage(command, encode(request), false);
  }

  struct Command {
    uint32_t command;
    bool isNotify;
    bool isResponse;
    binary_array_t buf;

    bool needReply() const;
  };

  bool readCommand(Command& cmd);

  void sendMessage(uint32_t command, const binary_array_t& out, bool needResponse);
  void sendReply(uint32_t command, const binary_array_t& out, int32_t returnCode);

  template <typename T>
  static bool decode(const binary_array_t& buf, T& value) {
    try {
      Common::MemoryInputStream stream(buf.data(), buf.size());
      KVBinaryInputStreamSerializer serializer(stream);
      serialize(value, serializer);
    } catch (std::exception&) {
      return false;
    }

    return true;
  }

  template <typename T>
  static binary_array_t encode(const T& value) {
    binary_array_t result;
    KVBinaryOutputStreamSerializer serializer;
    serialize(const_cast<T&>(value), serializer);
    Common::VectorOutputStream stream(result);
    serializer.dump(stream);
    return result;
  }

private:

  bool readStrict(uint8_t* ptr, size_t size);
  void writeStrict(const uint8_t* ptr, size_t size);
  System::TcpConnection& m_conn;
};

}
