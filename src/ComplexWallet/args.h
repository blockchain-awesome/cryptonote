#pragma once

#include <ctime>
#include <fstream>
#include <future>
#include <iomanip>
#include <thread>
#include <set>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "wallet_legacy/WalletLegacy.h"

#include "command_line/common.h"

using namespace cryptonote;
using namespace Logging;
using Common::JsonValue;

namespace cryptonote
{
extern const command_line::arg_descriptor<std::string> arg_wallet_file;
extern const command_line::arg_descriptor<std::string> arg_generate_new_wallet;
extern const command_line::arg_descriptor<std::string> arg_generate_wallet;
extern const command_line::arg_descriptor<std::string> arg_address;
extern const command_line::arg_descriptor<std::string> arg_send_key;
extern const command_line::arg_descriptor<std::string> arg_view_key;
extern const command_line::arg_descriptor<std::string> arg_bind_host;
extern const command_line::arg_descriptor<uint16_t> arg_bind_port;
extern const command_line::arg_descriptor<std::string> arg_daemon_address;
extern const command_line::arg_descriptor<std::string> arg_daemon_host;
extern const command_line::arg_descriptor<std::string> arg_password;
extern const command_line::arg_descriptor<uint16_t> arg_daemon_port;
extern const command_line::arg_descriptor<uint32_t> arg_log_level;
extern const command_line::arg_descriptor<bool> arg_testnet;
extern const command_line::arg_descriptor<std::vector<std::string>> arg_command;
extern bool parseUrlAddress(const std::string &url, std::string &address, uint16_t &port);
extern std::error_code initAndLoadWallet(IWalletLegacy &wallet, std::istream &walletFile, const std::string &password);
extern std::string tryToOpenWalletOrLoadKeysOrThrow(LoggerRef &logger, std::unique_ptr<IWalletLegacy> &wallet, const std::string &walletFile, const std::string &password);
extern std::string prepareWalletAddressFilename(const std::string &walletBaseName);
extern bool writeAddressFile(const std::string &addressFilename, const std::string &address);
extern std::string interpret_rpc_response(bool ok, const std::string &status);
extern JsonValue buildLoggerConfiguration(Level level, const std::string &logfile);
extern void printListTransfersHeader(LoggerRef &logger);
extern void printListTransfersItem(LoggerRef &logger, const WalletLegacyTransaction &txInfo, IWalletLegacy &wallet, const Currency &currency);

class TransferCommand
{

public:
  const cryptonote::Currency &m_currency;
  size_t fake_outs_count;
  std::vector<cryptonote::WalletLegacyTransfer> dsts;
  std::vector<uint8_t> extra;
  uint64_t fee;

  TransferCommand(const cryptonote::Currency &currency);

  bool parseArguments(LoggerRef &logger, const std::vector<std::string> &args);
};

} // namespace cryptonote
