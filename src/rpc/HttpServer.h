// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once 

#include <unordered_set>

#include <http/HttpRequest.h>
#include <http/HttpResponse.h>

#include <system/ContextGroup.h>
#include <system/Dispatcher.h>
#include <system/TcpListener.h>
#include <system/TcpConnection.h>
#include <system/Event.h>

#include <logging/LoggerRef.h>

namespace cryptonote {

class HttpServer {

public:

  HttpServer(System::Dispatcher& dispatcher, Logging::ILogger& log);

  void start(const std::string& address, uint16_t port);
  void stop();

  virtual void processRequest(const HttpRequest& request, HttpResponse& response) = 0;

protected:

  System::Dispatcher& m_dispatcher;

private:

  void acceptLoop();
  void connectionHandler(System::TcpConnection&& conn);

  System::ContextGroup workingContextGroup;
  Logging::LoggerRef logger;
  System::TcpListener m_listener;
  std::unordered_set<System::TcpConnection*> m_connections;
};

}
