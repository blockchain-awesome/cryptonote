// Copyright (c) 2018 The vig coin developers
//

#pragma once
#include <unordered_map>

#include "Common/JsonValue.h"
#include "JsonRpcServer/JsonRpcServer.h"
#include "Requests.h"
#include "Serialization/JsonInputValueSerializer.h"
#include "Serialization/JsonOutputStreamSerializer.h"

namespace MultiWalletService
{

class MultiWalletJsonRpcServer : public CryptoNote::JsonRpcServer
{
public:
  MultiWalletJsonRpcServer(System::Dispatcher &sys, System::Event &stopEvent, Logging::ILogger &loggerGroup);
  MultiWalletJsonRpcServer(const MultiWalletJsonRpcServer &) = delete;

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
  Logging::LoggerRef logger;

  std::error_code handleLogin(const Login::Request &request, Login::Response &response);
};

} // namespace MultiWalletService
