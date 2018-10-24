// #include "args.h"
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

#include "common/CommandLine.h"
#include "common/SignalHandler.h"
#include "common/StringTools.h"
#include "common/PathTools.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/protocol/handler.h"
#include "NodeRpcProxy/NodeRpcProxy.h"
#include "Rpc/CoreRpcServerCommandsDefinitions.h"
#include "Rpc/HttpClient.h"

#include "Wallet/WalletRpcServer.h"
#include "WalletLegacy/WalletLegacy.h"
#include "Wallet/LegacyKeysImporter.h"
#include "WalletLegacy/WalletHelper.h"

#include "version.h"

#include <Logging/LoggerManager.h>

#include "args.h"

#if defined(WIN32)
#include <crtdbg.h>
#endif

using namespace cryptonote;
using namespace Logging;
using Common::JsonValue;

namespace po = boost::program_options;

#define EXTENDED_LOGS_FILE "wallet_details.log"
#undef ERROR

namespace cryptonote
{

const command_line::arg_descriptor<std::string> arg_wallet_file = {"wallet-file", "Use wallet <arg>", ""};
const command_line::arg_descriptor<std::string> arg_generate_new_wallet = {"generate-new-wallet", "Generate new wallet and save it to <arg>", ""};
const command_line::arg_descriptor<std::string> arg_generate_wallet = {"generate-wallet", "Generate new wallet by keys", ""};
const command_line::arg_descriptor<std::string> arg_address = {"address", "Address", ""};
const command_line::arg_descriptor<std::string> arg_send_key = {"send-key", "Send private key", ""};
const command_line::arg_descriptor<std::string> arg_view_key = {"view-key", "View private key", ""};
const command_line::arg_descriptor<std::string> arg_bind_host = {"bind-host", "Complex Wallet host", "0.0.0.0"};
const command_line::arg_descriptor<uint16_t> arg_bind_port = {"bind-port", "Complex Wallet port", RPC_WALLET_PORT};
const command_line::arg_descriptor<std::string> arg_daemon_address = {"daemon-address", "Use daemon instance at <host>:<port>", ""};
const command_line::arg_descriptor<std::string> arg_daemon_host = {"daemon-host", "Use daemon instance at host <arg> instead of localhost", "localhost"};
const command_line::arg_descriptor<std::string> arg_password = {"password", "Wallet password", ""};
const command_line::arg_descriptor<uint16_t> arg_daemon_port = {"daemon-port", "Use daemon instance at port <arg> instead of 8081", RPC_DEFAULT_PORT};
const command_line::arg_descriptor<uint32_t> arg_log_level = {"set_log", "", INFO, true};
const command_line::arg_descriptor<bool> arg_testnet = {"testnet", "Used to deploy test nets. The daemon must be launched with --testnet flag", false};
const command_line::arg_descriptor<std::vector<std::string>> arg_command = {"command", ""};

std::string interpret_rpc_response(bool ok, const std::string &status)
{
  std::string err;
  if (ok)
  {
    if (status == CORE_RPC_STATUS_BUSY)
    {
      err = "daemon is busy. Please try later";
    }
    else if (status != CORE_RPC_STATUS_OK)
    {
      err = status;
    }
  }
  else
  {
    err = "possible lost connection to daemon";
  }
  return err;
}

template <typename IterT, typename ValueT = typename IterT::value_type>
class ArgumentReader
{
public:
  ArgumentReader(IterT begin, IterT end) : m_begin(begin), m_end(end), m_cur(begin)
  {
  }

  bool eof() const
  {
    return m_cur == m_end;
  }

  ValueT next()
  {
    if (eof())
    {
      throw std::runtime_error("unexpected end of arguments");
    }

    return *m_cur++;
  }

private:
  IterT m_cur;
  IterT m_begin;
  IterT m_end;
};

TransferCommand::TransferCommand(const cryptonote::Currency &currency) : m_currency(currency), fake_outs_count(0), fee(currency.minimumFee())
{
}

bool TransferCommand::parseArguments(LoggerRef &logger, const std::vector<std::string> &args)
{

  ArgumentReader<std::vector<std::string>::const_iterator> ar(args.begin(), args.end());

  try
  {

    auto mixin_str = ar.next();

    if (!Common::fromString(mixin_str, fake_outs_count))
    {
      logger(ERROR, BRIGHT_RED) << "mixin_count should be non-negative integer, got " << mixin_str;
      return false;
    }

    while (!ar.eof())
    {

      auto arg = ar.next();

      if (arg.size() && arg[0] == '-')
      {

        const auto &value = ar.next();

        if (arg == "-p")
        {
          if (!createTxExtraWithPaymentId(value, extra))
          {
            logger(ERROR, BRIGHT_RED) << "payment ID has invalid format: \"" << value << "\", expected 64-character string";
            return false;
          }
        }
        else if (arg == "-f")
        {
          bool ok = m_currency.parseAmount(value, fee);
          if (!ok)
          {
            logger(ERROR, BRIGHT_RED) << "Fee value is invalid: " << value;
            return false;
          }

          if (fee < m_currency.minimumFee())
          {
            logger(ERROR, BRIGHT_RED) << "Fee value is less than minimum: " << m_currency.minimumFee();
            return false;
          }
        }
      }
      else
      {
        WalletLegacyTransfer destination;
        cryptonote::TransactionDestinationEntry de;

        if (!m_currency.parseAccountAddressString(arg, de.addr))
        {
          crypto::Hash paymentId;
          if (cryptonote::parsePaymentId(arg, paymentId))
          {
            logger(ERROR, BRIGHT_RED) << "Invalid payment ID usage. Please, use -p <payment_id>. See help for details.";
          }
          else
          {
            logger(ERROR, BRIGHT_RED) << "Wrong address: " << arg;
          }

          return false;
        }

        auto value = ar.next();
        bool ok = m_currency.parseAmount(value, de.amount);
        if (!ok || 0 == de.amount)
        {
          logger(ERROR, BRIGHT_RED) << "amount is wrong: " << arg << ' ' << value << ", expected number from 0 to " << m_currency.formatAmount(std::numeric_limits<uint64_t>::max());
          return false;
        }
        destination.address = arg;
        destination.amount = de.amount;

        dsts.push_back(destination);
      }
    }

    if (dsts.empty())
    {
      logger(ERROR, BRIGHT_RED) << "At least one destination address is required";
      return false;
    }
  }
  catch (const std::exception &e)
  {
    logger(ERROR, BRIGHT_RED) << e.what();
    return false;
  }

  return true;
}

std::error_code initAndLoadWallet(IWalletLegacy &wallet, std::istream &walletFile, const std::string &password)
{
  WalletHelper::InitWalletResultObserver initObserver;
  std::future<std::error_code> f_initError = initObserver.initResult.get_future();

  WalletHelper::IWalletRemoveObserverGuard removeGuard(wallet, initObserver);
  wallet.initAndLoad(walletFile, password);
  auto initError = f_initError.get();

  return initError;
}

std::string makeCenteredString(size_t width, const std::string &text)
{
  if (text.size() >= width)
  {
    return text;
  }

  size_t offset = (width - text.size() + 1) / 2;
  return std::string(offset, ' ') + text + std::string(width - text.size() - offset, ' ');
}

const size_t TIMESTAMP_MAX_WIDTH = 19;
const size_t HASH_MAX_WIDTH = 64;
const size_t TOTAL_AMOUNT_MAX_WIDTH = 20;
const size_t FEE_MAX_WIDTH = 14;
const size_t BLOCK_MAX_WIDTH = 7;
const size_t UNLOCK_TIME_MAX_WIDTH = 11;

void printListTransfersHeader(LoggerRef &logger)
{
  std::string header = makeCenteredString(TIMESTAMP_MAX_WIDTH, "timestamp (UTC)") + "  ";
  header += makeCenteredString(HASH_MAX_WIDTH, "hash") + "  ";
  header += makeCenteredString(TOTAL_AMOUNT_MAX_WIDTH, "total amount") + "  ";
  header += makeCenteredString(FEE_MAX_WIDTH, "fee") + "  ";
  header += makeCenteredString(BLOCK_MAX_WIDTH, "block") + "  ";
  header += makeCenteredString(UNLOCK_TIME_MAX_WIDTH, "unlock time");

  logger(INFO) << header;
  logger(INFO) << std::string(header.size(), '-');
}

void printListTransfersItem(LoggerRef &logger, const WalletLegacyTransaction &txInfo, IWalletLegacy &wallet, const Currency &currency)
{
  std::vector<uint8_t> extraVec = Common::asBinaryArray(txInfo.extra);

  crypto::Hash paymentId;
  std::string paymentIdStr = (getPaymentIdFromTxExtra(extraVec, paymentId) && paymentId != NULL_HASH ? Common::podToHex(paymentId) : "");

  char timeString[TIMESTAMP_MAX_WIDTH + 1];
  time_t timestamp = static_cast<time_t>(txInfo.timestamp);
  if (std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", std::gmtime(&timestamp)) == 0)
  {
    throw std::runtime_error("time buffer is too small");
  }

  std::string rowColor = txInfo.totalAmount < 0 ? MAGENTA : GREEN;
  logger(INFO, rowColor)
      << std::setw(TIMESTAMP_MAX_WIDTH) << timeString
      << "  " << std::setw(HASH_MAX_WIDTH) << Common::podToHex(txInfo.hash)
      << "  " << std::setw(TOTAL_AMOUNT_MAX_WIDTH) << currency.formatAmount(txInfo.totalAmount)
      << "  " << std::setw(FEE_MAX_WIDTH) << currency.formatAmount(txInfo.fee)
      << "  " << std::setw(BLOCK_MAX_WIDTH) << txInfo.blockHeight
      << "  " << std::setw(UNLOCK_TIME_MAX_WIDTH) << txInfo.unlockTime;

  if (!paymentIdStr.empty())
  {
    logger(INFO, rowColor) << "payment ID: " << paymentIdStr;
  }

  if (txInfo.totalAmount < 0)
  {
    if (txInfo.transferCount > 0)
    {
      logger(INFO, rowColor) << "transfers:";
      for (TransferId id = txInfo.firstTransferId; id < txInfo.firstTransferId + txInfo.transferCount; ++id)
      {
        WalletLegacyTransfer tr;
        wallet.getTransfer(id, tr);
        logger(INFO, rowColor) << tr.address << "  " << std::setw(TOTAL_AMOUNT_MAX_WIDTH) << currency.formatAmount(tr.amount);
      }
    }
  }

  logger(INFO, rowColor) << " "; //just to make logger print one endline
}

std::string prepareWalletAddressFilename(const std::string &walletBaseName)
{
  return walletBaseName + ".address";
}

bool writeAddressFile(const std::string &addressFilename, const std::string &address)
{
  std::ofstream addressFile(addressFilename, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!addressFile.good())
  {
    return false;
  }

  addressFile << address;

  return true;
}

bool parseUrlAddress(const std::string &url, std::string &address, uint16_t &port)
{
  auto pos = url.find("://");
  size_t addrStart = 0;

  if (pos != std::string::npos)
  {
    addrStart = pos + 3;
  }

  auto addrEnd = url.find(':', addrStart);

  if (addrEnd != std::string::npos)
  {
    auto portEnd = url.find('/', addrEnd);
    port = Common::fromString<uint16_t>(url.substr(
        addrEnd + 1, portEnd == std::string::npos ? std::string::npos : portEnd - addrEnd - 1));
  }
  else
  {
    addrEnd = url.find('/');
    port = 80;
  }

  address = url.substr(addrStart, addrEnd - addrStart);
  return true;
}

JsonValue buildLoggerConfiguration(Level level, const std::string &logfile)
{
  JsonValue loggerConfiguration(JsonValue::OBJECT);
  loggerConfiguration.insert("globalLevel", static_cast<int64_t>(level));

  JsonValue &cfgLoggers = loggerConfiguration.insert("loggers", JsonValue::ARRAY);

  JsonValue &consoleLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
  consoleLogger.insert("type", "console");
  consoleLogger.insert("level", static_cast<int64_t>(TRACE));
  consoleLogger.insert("pattern", "");

  JsonValue &fileLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
  fileLogger.insert("type", "file");
  fileLogger.insert("filename", logfile);
  fileLogger.insert("level", static_cast<int64_t>(TRACE));

  return loggerConfiguration;
}

std::string tryToOpenWalletOrLoadKeysOrThrow(LoggerRef &logger, std::unique_ptr<IWalletLegacy> &wallet, const std::string &walletFile, const std::string &password)
{
  std::string walletFileName = walletFile;
  boost::system::error_code ignore;
  bool walletExists = boost::filesystem::exists(walletFileName, ignore);
  if (!walletExists)
  {
    throw std::runtime_error("File '" + walletFile + "' not exist!");
  }

  logger(INFO) << "Loading wallet...";
  std::ifstream ifs;
  ifs.open(walletFileName, std::ios_base::binary | std::ios_base::in);
  if (ifs.fail())
  {
    throw std::runtime_error("error opening wallet file '" + walletFileName + "'");
  }

  initAndLoadWallet(*wallet, ifs, password);

  ifs.close();
  return walletFileName;
}

} // namespace cryptonote