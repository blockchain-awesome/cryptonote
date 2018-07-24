// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "WalletRPCService.h"
#include <functional>
#include "Serialization/JsonInputValueSerializer.h"
#include "Serialization/JsonOutputStreamSerializer.h"

#include "Errors.h"

#include <iostream>

using namespace std;

namespace MultiWalletService {

MultiWalletJsonRpcServer::MultiWalletJsonRpcServer(System::Dispatcher& sys, System::Event& stopEvent, Logging::ILogger& loggerGroup) 
  : JsonRpcServer(sys, stopEvent, loggerGroup)
  , logger(loggerGroup, "MultiWalletJsonRpcServer")
{
  // handlers.emplace("login", jsonHandler<Login::Request, Login::Response>(std::bind(&MultiWalletJsonRpcServer::handleLogin, this, std::placeholders::_1, std::placeholders::_2)));
}

void MultiWalletJsonRpcServer::processJsonRpcRequest(const Common::JsonValue& req, Common::JsonValue& resp) {
  try {
    prepareJsonResponse(req, resp);

    if (!req.contains("method")) {
      logger(Logging::WARNING) << "Field \"method\" is not found in json request: " << req;
      makeGenericErrorReponse(resp, "Invalid Request", -3600);
      return;
    }

    if (!req("method").isString()) {
      logger(Logging::WARNING) << "Field \"method\" is not a string type: " << req;
      makeGenericErrorReponse(resp, "Invalid Request", -3600);
      return;
    }

    std::string method = req("method").getString();

    auto it = handlers.find(method);
    if (it == handlers.end()) {
      logger(Logging::WARNING) << "Requested method not found: " << method;
      makeMethodNotFoundResponse(resp);
      return;
    }

    logger(Logging::DEBUGGING) << method << " request came";

    Common::JsonValue params(Common::JsonValue::OBJECT);
    if (req.contains("params")) {
      params = req("params");
    }

    it->second(params, resp);
  } catch (std::exception& e) {
    logger(Logging::WARNING) << "Error occurred while processing JsonRpc request: " << e.what();
    makeGenericErrorReponse(resp, e.what());
  }
}

std::error_code MultiWalletJsonRpcServer::handleLogin(const Login::Request& request, Login::Response& response) {

  cout << request.address << endl;
  cout << request.spendPublicKey << endl;
  cout << request.spendPublicKey << endl;
  error_code ec = make_error_code(ErrorCode::OK);
  return ec;
}

}
