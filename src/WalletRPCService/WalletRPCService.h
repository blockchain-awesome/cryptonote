// Copyright (c) 2018 The vig coin developers
//

#pragma once
#include <unordered_map>
#include <boost/program_options/variables_map.hpp>

#include "Common/JsonValue.h"
#include "JsonRpcServer/JsonRpcServer.h"
#include "Requests.h"
#include "Serialization/JsonInputValueSerializer.h"
#include "Serialization/JsonOutputStreamSerializer.h"

#include "IWalletLegacy.h"
#include "CommandParser.h"

#include "Common/ConsoleHandler.h"
#include "CryptoNoteCore/CryptoNoteBasicImpl.h"
#include "CryptoNoteCore/Currency.h"
#include "NodeRpcProxy/NodeRpcProxy.h"

#include <Logging/LoggerRef.h>
#include <Logging/LoggerManager.h>

#include <System/Dispatcher.h>
#include <System/Ipv4Address.h>

namespace MultiWalletService
{

class MultiWalletJsonRpcServer : public CryptoNote::JsonRpcServer
{
public:
  MultiWalletJsonRpcServer(System::Dispatcher &sys, System::Event &stopEvent, const CryptoNote::Currency &currency, Logging::ILogger &loggerGroup);
  MultiWalletJsonRpcServer(const MultiWalletJsonRpcServer &) = delete;
  std::string getCommands();
  void init(const boost::program_options::variables_map &vm);
  void handle_command_line(const boost::program_options::variables_map &vm);

protected:
  virtual void processJsonRpcRequest(const Common::JsonValue &req, Common::JsonValue &resp) override;

  typedef std::function<void(const Common::JsonValue &jsonRpcParams, Common::JsonValue &jsonResponse)> HandlerFunction;

  template <typename RequestType, typename ResponseType, typename RequestHandler>
  HandlerFunction jsonHandler(RequestHandler handler)
  {
    return [handler](const Common::JsonValue &jsonRpcParams, Common::JsonValue &jsonResponse) mutable {
      RequestType request;
      ResponseType response;

      try
      {
        CryptoNote::JsonInputValueSerializer inputSerializer(const_cast<Common::JsonValue &>(jsonRpcParams));
        serialize(request, inputSerializer);
      }
      catch (std::exception &)
      {
        makeGenericErrorReponse(jsonResponse, "Invalid Request", -32600);
        return;
      }

      std::error_code ec = handler(request, response);
      if (ec)
      {
        makeErrorResponse(ec, jsonResponse);
        return;
      }

      CryptoNote::JsonOutputStreamSerializer outputSerializer;
      serialize(response, outputSerializer);
      fillJsonResponse(outputSerializer.getValue(), jsonResponse);
    };
  }

  std::unordered_map<std::string, HandlerFunction> handlers;

private:
  std::string m_daemon_address;
  std::string m_daemon_host;
  uint16_t m_daemon_port;
  std::string m_bind_ip;
  uint16_t m_bind_port;

  Common::ConsoleHandler m_consoleHandler;
  const CryptoNote::Currency &m_currency;
  // Logging::LoggerManager &logManager;
  System::Dispatcher &m_dispatcher;
  Logging::LoggerRef logger;

  std::unique_ptr<CryptoNote::NodeRpcProxy> m_node;

  std::error_code handleLogin(const Login::Request &request, Login::Response &response);
};

} // namespace MultiWalletService
