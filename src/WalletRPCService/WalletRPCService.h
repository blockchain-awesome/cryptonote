// Copyright (c) 2018 The vig coin developers
//

#pragma once
#include <unordered_map>

#include "Common/JsonValue.h"
#include "JsonRpcServer/JsonRpcServer.h"
#include "PaymentServiceJsonRpcMessages.h"
#include "Serialization/JsonInputValueSerializer.h"
#include "Serialization/JsonOutputStreamSerializer.h"

namespace MultiWalletService
{

class MultiWalletJsonRpcServer : public CryptoNote::JsonRpcServer
{
public:
  MultiWalletJsonRpcServer(System::Dispatcher &sys, System::Event &stopEvent, WalletService &service, Logging::ILogger &loggerGroup);
  MultiWalletJsonRpcServer(const MultiWalletJsonRpcServer &) = delete;

protected:
  virtual void processJsonRpcRequest(const Common::JsonValue &req, Common::JsonValue &resp) override;
}

} // namespace WalletService
