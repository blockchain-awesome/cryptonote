#pragma once
#include "command_line/options.h"
#include <logging/LoggerManager.h>
#include "wallet_legacy/WalletLegacy.h"

using namespace cryptonote;
using namespace Logging;

extern const command_line::arg_descriptor<std::string> arg_wallet_file;
extern const command_line::arg_descriptor<std::string> arg_generate_new_wallet;
extern const command_line::arg_descriptor<std::string> arg_daemon_address;
extern const command_line::arg_descriptor<std::string> arg_daemon_host;
extern const command_line::arg_descriptor<std::string> arg_password;
extern const command_line::arg_descriptor<uint16_t> arg_daemon_port;
extern const command_line::arg_descriptor<uint32_t> arg_log_level;
extern const command_line::arg_descriptor<bool> arg_testnet;
extern const command_line::arg_descriptor< std::vector<std::string> > arg_command;

extern bool parseUrlAddress(const std::string& url, std::string& address, uint16_t& port);

extern std::string tryToOpenWalletOrLoadKeysOrThrow(LoggerRef& logger, std::unique_ptr<IWalletLegacy>& wallet, const std::string& walletFile, const std::string& password);
