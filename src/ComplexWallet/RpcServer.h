// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <unordered_map>

#include "common/JsonValue.h"
#include "JsonRpcServer/JsonRpcServer.h"
#include "JsonRpcParameters.h"
#include "serialization/JsonInputValueSerializer.h"
#include "serialization/JsonOutputStreamSerializer.h"

#include "cryptonote.h"
#include "WalletManager.h"

namespace ComplexWallet
{

class ComplexWalletServer : public cryptonote::JsonRpcServer
{
public:
  ComplexWalletServer(System::Dispatcher &sys, System::Event &stopEvent, Logging::ILogger &loggerGroup, WalletManager &wallet);
  ComplexWalletServer(const JsonRpcServer &) = delete;

  std::string getAddress(const std::string &token);

  void init(const INode::Callback& callback);

  virtual void start(const std::string &bindAddress, uint16_t bindPort) override;

protected:
  virtual void processJsonRpcRequest(const Common::JsonValue &req, Common::JsonValue &resp) override;

private:
  Logging::LoggerRef logger;
  std::mutex m_mutex;
  std::thread m_workerThread;
  WalletManager &m_wallet;

  std::map<std::string, std::string> m_tokenMap;
  std::map<std::string, std::time_t> m_tokenTime;

  typedef std::function<void(const Common::JsonValue &jsonRpcParams, Common::JsonValue &jsonResponse)> HandlerFunction;

  template <typename RequestType, typename ResponseType, typename RequestHandler>
  HandlerFunction jsonHandler(RequestHandler handler)
  {
    return [handler](const Common::JsonValue &jsonRpcParams, Common::JsonValue &jsonResponse) mutable {
      RequestType request;
      ResponseType response;

      try
      {
        cryptonote::JsonInputValueSerializer inputSerializer(const_cast<Common::JsonValue &>(jsonRpcParams));
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

      cryptonote::JsonOutputStreamSerializer outputSerializer;
      serialize(response, outputSerializer);
      fillJsonResponse(outputSerializer.getValue(), jsonResponse);
    };
  }

  std::unordered_map<std::string, HandlerFunction> handlers;

  std::error_code handleLogin(const Login::Request &request, Login::Response &response);
  std::error_code handleGetBalance(const GetBalance::Request &request, GetBalance::Response &response);

  // std::error_code handleGetBlockHashes(const GetBlockHashes::Request& request, GetBlockHashes::Response& response);
  // std::error_code handleGetTransactionHashes(const GetTransactionHashes::Request& request, GetTransactionHashes::Response& response);
  // std::error_code handleGetTransactions(const GetTransactions::Request& request, GetTransactions::Response& response);
  // std::error_code handleGetUnconfirmedTransactionHashes(const GetUnconfirmedTransactionHashes::Request& request, GetUnconfirmedTransactionHashes::Response& response);
  // std::error_code handleGetTransaction(const GetTransaction::Request& request, GetTransaction::Response& response);
  // std::error_code handleSendTransaction(const SendTransaction::Request& request, SendTransaction::Response& response);
  // std::error_code handleCreateDelayedTransaction(const CreateDelayedTransaction::Request& request, CreateDelayedTransaction::Response& response);
  // std::error_code handleGetDelayedTransactionHashes(const GetDelayedTransactionHashes::Request& request, GetDelayedTransactionHashes::Response& response);
  // std::error_code handleDeleteDelayedTransaction(const DeleteDelayedTransaction::Request& request, DeleteDelayedTransaction::Response& response);
  // std::error_code handleSendDelayedTransaction(const SendDelayedTransaction::Request& request, SendDelayedTransaction::Response& response);
  // std::error_code handleGetViewKey(const GetViewKey::Request& request, GetViewKey::Response& response);
  // std::error_code handleGetStatus(const GetStatus::Request& request, GetStatus::Response& response);
  // std::error_code handleGetAddresses(const GetAddresses::Request& request, GetAddresses::Response& response);
};

} // namespace ComplexWallet
