// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ComplexWallet.h"

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

#include "Common/CommandLine.h"
#include "Common/SignalHandler.h"
#include "Common/StringTools.h"
#include "Common/PathTools.h"
#include "Common/Util.h"
#include "CryptoNoteCore/CryptoNoteFormatUtils.h"
#include "CryptoNoteProtocol/CryptoNoteProtocolHandler.h"
#include "NodeRpcProxy/NodeRpcProxy.h"
#include "Rpc/CoreRpcServerCommandsDefinitions.h"
#include "Rpc/HttpClient.h"

#include "Wallet/WalletRpcServer.h"
#include "Wallet/LegacyKeysImporter.h"
#include "WalletLegacy/WalletHelper.h"

#include "version.h"

#include <Logging/LoggerManager.h>

#include "args.h"

#include "SingleWallet.h"


#if defined(WIN32)
#include <crtdbg.h>
#endif

using namespace CryptoNote;
using namespace Logging;
using Common::JsonValue;

using namespace ComplexWallet;

namespace po = boost::program_options;

#define EXTENDED_LOGS_FILE "wallet_details.log"
#undef ERROR

namespace CryptoNote
{

std::string complex_wallet::get_commands_str()
{
  std::stringstream ss;
  ss << "Commands: " << ENDL;
  std::string usage = m_consoleHandler.getUsage();
  boost::replace_all(usage, "\n", "\n  ");
  usage.insert(0, "  ");
  ss << usage << ENDL;
  return ss.str();
}

bool complex_wallet::help(const std::vector<std::string> &args /* = std::vector<std::string>()*/)
{
  success_msg_writer() << get_commands_str();
  return true;
}

bool complex_wallet::exit(const std::vector<std::string> &args)
{
  m_consoleHandler.requestStop();
  return true;
}

complex_wallet::complex_wallet(System::Dispatcher &dispatcher, const CryptoNote::Currency &currency, Logging::LoggerManager &log) : m_dispatcher(dispatcher),
                                                                                                                                    m_daemon_port(0),
                                                                                                                                    m_currency(currency),
                                                                                                                                    logManager(log),
                                                                                                                                    logger(log, "complexwallet"),
                                                                                                                                    m_refresh_progress_reporter(*this),
                                                                                                                                    m_initResultPromise(nullptr),
                                                                                                                                    m_walletSynchronized(false)
{
  m_consoleHandler.setHandler("start_mining", boost::bind(&complex_wallet::start_mining, this, _1), "start_mining [<number_of_threads>] - Start mining in daemon");
  m_consoleHandler.setHandler("stop_mining", boost::bind(&complex_wallet::stop_mining, this, _1), "Stop mining in daemon");
  //m_consoleHandler.setHandler("refresh", boost::bind(&complex_wallet::refresh, this, _1), "Resynchronize transactions and balance");
  m_consoleHandler.setHandler("balance", boost::bind(&complex_wallet::show_balance, this, _1), "Show current wallet balance");
  m_consoleHandler.setHandler("incoming_transfers", boost::bind(&complex_wallet::show_incoming_transfers, this, _1), "Show incoming transfers");
  m_consoleHandler.setHandler("list_transfers", boost::bind(&complex_wallet::listTransfers, this, _1), "Show all known transfers");
  m_consoleHandler.setHandler("payments", boost::bind(&complex_wallet::show_payments, this, _1), "payments <payment_id_1> [<payment_id_2> ... <payment_id_N>] - Show payments <payment_id_1>, ... <payment_id_N>");
  m_consoleHandler.setHandler("bc_height", boost::bind(&complex_wallet::show_blockchain_height, this, _1), "Show blockchain height");
  m_consoleHandler.setHandler("transfer", boost::bind(&complex_wallet::transfer, this, _1),
                              "transfer <mixin_count> <addr_1> <amount_1> [<addr_2> <amount_2> ... <addr_N> <amount_N>] [-p payment_id] [-f fee]"
                              " - Transfer <amount_1>,... <amount_N> to <address_1>,... <address_N>, respectively. "
                              "<mixin_count> is the number of transactions yours is indistinguishable from (from 0 to maximum available)");
  m_consoleHandler.setHandler("set_log", boost::bind(&complex_wallet::set_log, this, _1), "set_log <level> - Change current log level, <level> is a number 0-4");
  m_consoleHandler.setHandler("address", boost::bind(&complex_wallet::print_address, this, _1), "Show current wallet public address");
  m_consoleHandler.setHandler("save", boost::bind(&complex_wallet::save, this, _1), "Save wallet synchronized data");
  m_consoleHandler.setHandler("reset", boost::bind(&complex_wallet::reset, this, _1), "Discard cache data and start synchronizing from the start");
  m_consoleHandler.setHandler("help", boost::bind(&complex_wallet::help, this, _1), "Show this help");
  m_consoleHandler.setHandler("exit", boost::bind(&complex_wallet::exit, this, _1), "Close wallet");
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::set_log(const std::vector<std::string> &args)
{
  if (args.size() != 1)
  {
    fail_msg_writer() << "use: set_log <log_level_number_0-4>";
    return true;
  }

  uint16_t l = 0;
  if (!Common::fromString(args[0], l))
  {
    fail_msg_writer() << "wrong number format, use: set_log <log_level_number_0-4>";
    return true;
  }

  if (l > Logging::TRACE)
  {
    fail_msg_writer() << "wrong number range, use: set_log <log_level_number_0-4>";
    return true;
  }

  logManager.setMaxLevel(static_cast<Logging::Level>(l));
  return true;
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::init(const boost::program_options::variables_map &vm)
{
  handle_command_line(vm);

  std::string walletFileName;

  Tools::PasswordContainer pwd_container;
  if (command_line::has_arg(vm, arg_password))
  {
    pwd_container.password(command_line::get_arg(vm, arg_password));
  }
  else if (!pwd_container.read_password())
  {
    fail_msg_writer() << "failed to read wallet password";
    return false;
  }

  this->m_node.reset(new NodeRpcProxy(m_daemon_host, m_daemon_port));

  std::promise<std::error_code> errorPromise;
  std::future<std::error_code> f_error = errorPromise.get_future();
  auto callback = [&errorPromise](std::error_code e) { errorPromise.set_value(e); };

  m_node->addObserver(static_cast<INodeRpcProxyObserver *>(this));
  m_node->init(callback);
  auto error = f_error.get();
  if (error)
  {
    fail_msg_writer() << "failed to init NodeRPCProxy: " << error.message();
    return false;
  }
  m_wallet.reset(new SingleWallet(m_currency, *m_node));

  try
  {
    m_wallet_file = tryToOpenWalletOrLoadKeysOrThrow(logger, m_wallet, m_wallet_file_arg, pwd_container.password());
  }
  catch (const std::exception &e)
  {
    fail_msg_writer() << "failed to load wallet: " << e.what();
    return false;
  }

  m_wallet->addObserver(this);
  m_node->addObserver(static_cast<INodeObserver *>(this));

  logger(INFO, BRIGHT_WHITE) << "Opened wallet: " << m_wallet->getAddress();

  success_msg_writer() << "**********************************************************************\n"
                       << "Use \"help\" command to see the list of available commands.\n"
                       << "**********************************************************************";
  return true;
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::deinit()
{
  m_wallet->removeObserver(this);
  m_node->removeObserver(static_cast<INodeObserver *>(this));
  m_node->removeObserver(static_cast<INodeRpcProxyObserver *>(this));

  if (!m_wallet.get())
    return true;

  return close_wallet();
}
//----------------------------------------------------------------------------------------------------
void complex_wallet::handle_command_line(const boost::program_options::variables_map &vm)
{
  m_wallet_file_arg = command_line::get_arg(vm, arg_wallet_file);
  m_generate_new = command_line::get_arg(vm, arg_generate_new_wallet);
  m_daemon_address = command_line::get_arg(vm, arg_daemon_address);
  m_daemon_host = command_line::get_arg(vm, arg_daemon_host);
  m_daemon_port = command_line::get_arg(vm, arg_daemon_port);
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::new_wallet(const std::string &wallet_file, const std::string &password)
{
  m_wallet_file = wallet_file;

  m_wallet.reset(new SingleWallet(m_currency, *m_node.get()));
  m_node->addObserver(static_cast<INodeObserver *>(this));
  m_wallet->addObserver(this);
  try
  {
    m_initResultPromise.reset(new std::promise<std::error_code>());
    std::future<std::error_code> f_initError = m_initResultPromise->get_future();
    m_wallet->initAndGenerate(password);
    auto initError = f_initError.get();
    m_initResultPromise.reset(nullptr);
    if (initError)
    {
      fail_msg_writer() << "failed to generate new wallet: " << initError.message();
      return false;
    }

    try
    {
      CryptoNote::WalletHelper::storeWallet(*m_wallet, m_wallet_file);
    }
    catch (std::exception &e)
    {
      fail_msg_writer() << "failed to save new wallet: " << e.what();
      throw;
    }

    AccountKeys keys;
    m_wallet->getAccountKeys(keys);

    logger(INFO, BRIGHT_WHITE) << "Generated new wallet: " << m_wallet->getAddress() << std::endl
                               << "view key: " << Common::podToHex(keys.viewSecretKey);
  }
  catch (const std::exception &e)
  {
    fail_msg_writer() << "failed to generate new wallet: " << e.what();
    return false;
  }

  success_msg_writer() << "**********************************************************************\n"
                       << "Your wallet has been generated.\n"
                       << "Use \"help\" command to see the list of available commands.\n"
                       << "Always use \"exit\" command when closing complexwallet to save\n"
                       << "current session's state. Otherwise, you will possibly need to synchronize \n"
                       << "your wallet again. Your wallet key is NOT under risk anyway.\n"
                       << "**********************************************************************";
  return true;
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::close_wallet()
{
  try
  {
    CryptoNote::WalletHelper::storeWallet(*m_wallet, m_wallet_file);
  }
  catch (const std::exception &e)
  {
    fail_msg_writer() << e.what();
    return false;
  }

  m_wallet->removeObserver(this);
  m_wallet->shutdown();

  return true;
}

//----------------------------------------------------------------------------------------------------
bool complex_wallet::save(const std::vector<std::string> &args)
{
  try
  {
    CryptoNote::WalletHelper::storeWallet(*m_wallet, m_wallet_file);
    success_msg_writer() << "Wallet data saved";
  }
  catch (const std::exception &e)
  {
    fail_msg_writer() << e.what();
  }

  return true;
}

bool complex_wallet::reset(const std::vector<std::string> &args)
{
  {
    std::unique_lock<std::mutex> lock(m_walletSynchronizedMutex);
    m_walletSynchronized = false;
  }

  m_wallet->reset();
  success_msg_writer(true) << "Reset completed successfully.";

  std::unique_lock<std::mutex> lock(m_walletSynchronizedMutex);
  while (!m_walletSynchronized)
  {
    m_walletSynchronizedCV.wait(lock);
  }

  std::cout << std::endl;

  return true;
}

bool complex_wallet::start_mining(const std::vector<std::string> &args)
{
  COMMAND_RPC_START_MINING::request req;
  req.miner_address = m_wallet->getAddress();

  bool ok = true;
  size_t max_mining_threads_count = (std::max)(std::thread::hardware_concurrency(), static_cast<unsigned>(2));
  if (0 == args.size())
  {
    req.threads_count = 1;
  }
  else if (1 == args.size())
  {
    uint16_t num = 1;
    ok = Common::fromString(args[0], num);
    ok = ok && (1 <= num && num <= max_mining_threads_count);
    req.threads_count = num;
  }
  else
  {
    ok = false;
  }

  if (!ok)
  {
    fail_msg_writer() << "invalid arguments. Please use start_mining [<number_of_threads>], "
                      << "<number_of_threads> should be from 1 to " << max_mining_threads_count;
    return true;
  }

  COMMAND_RPC_START_MINING::response res;

  try
  {
    HttpClient httpClient(m_dispatcher, m_daemon_host, m_daemon_port);

    invokeJsonCommand(httpClient, "/start_mining", req, res);

    std::string err = interpret_rpc_response(true, res.status);
    if (err.empty())
      success_msg_writer() << "Mining started in daemon";
    else
      fail_msg_writer() << "mining has NOT been started: " << err;
  }
  catch (const ConnectException &)
  {
    printConnectionError();
  }
  catch (const std::exception &e)
  {
    fail_msg_writer() << "Failed to invoke rpc method: " << e.what();
  }

  return true;
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::stop_mining(const std::vector<std::string> &args)
{
  COMMAND_RPC_STOP_MINING::request req;
  COMMAND_RPC_STOP_MINING::response res;

  try
  {
    HttpClient httpClient(m_dispatcher, m_daemon_host, m_daemon_port);

    invokeJsonCommand(httpClient, "/stop_mining", req, res);
    std::string err = interpret_rpc_response(true, res.status);
    if (err.empty())
      success_msg_writer() << "Mining stopped in daemon";
    else
      fail_msg_writer() << "mining has NOT been stopped: " << err;
  }
  catch (const ConnectException &)
  {
    printConnectionError();
  }
  catch (const std::exception &e)
  {
    fail_msg_writer() << "Failed to invoke rpc method: " << e.what();
  }

  return true;
}
//----------------------------------------------------------------------------------------------------
void complex_wallet::initCompleted(std::error_code result)
{
  if (m_initResultPromise.get() != nullptr)
  {
    m_initResultPromise->set_value(result);
  }
}
//----------------------------------------------------------------------------------------------------
void complex_wallet::connectionStatusUpdated(bool connected)
{
  if (connected)
  {
    logger(INFO, GREEN) << "Wallet connected to daemon.";
  }
  else
  {
    printConnectionError();
  }
}
//----------------------------------------------------------------------------------------------------
void complex_wallet::externalTransactionCreated(CryptoNote::TransactionId transactionId)
{
  WalletLegacyTransaction txInfo;
  m_wallet->getTransaction(transactionId, txInfo);

  std::stringstream logPrefix;
  if (txInfo.blockHeight == WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT)
  {
    logPrefix << "Unconfirmed";
  }
  else
  {
    logPrefix << "Height " << txInfo.blockHeight << ',';
  }

  if (txInfo.totalAmount >= 0)
  {
    logger(INFO, GREEN) << logPrefix.str() << " transaction " << Common::podToHex(txInfo.hash) << ", received " << m_currency.formatAmount(txInfo.totalAmount);
  }
  else
  {
    logger(INFO, MAGENTA) << logPrefix.str() << " transaction " << Common::podToHex(txInfo.hash) << ", spent " << m_currency.formatAmount(static_cast<uint64_t>(-txInfo.totalAmount));
  }

  if (txInfo.blockHeight == WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT)
  {
    m_refresh_progress_reporter.update(m_node->getLastLocalBlockHeight(), true);
  }
  else
  {
    m_refresh_progress_reporter.update(txInfo.blockHeight, true);
  }
}
//----------------------------------------------------------------------------------------------------
void complex_wallet::synchronizationCompleted(std::error_code result)
{
  std::unique_lock<std::mutex> lock(m_walletSynchronizedMutex);
  m_walletSynchronized = true;
  m_walletSynchronizedCV.notify_one();
}

void complex_wallet::synchronizationProgressUpdated(uint32_t current, uint32_t total)
{
  std::unique_lock<std::mutex> lock(m_walletSynchronizedMutex);
  if (!m_walletSynchronized)
  {
    m_refresh_progress_reporter.update(current, false);
  }
}

bool complex_wallet::show_balance(const std::vector<std::string> &args /* = std::vector<std::string>()*/)
{
  success_msg_writer() << "available balance: " << m_currency.formatAmount(m_wallet->actualBalance()) << ", locked amount: " << m_currency.formatAmount(m_wallet->pendingBalance());

  return true;
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::show_incoming_transfers(const std::vector<std::string> &args)
{
  bool hasTransfers = false;
  size_t transactionsCount = m_wallet->getTransactionCount();
  for (size_t trantransactionNumber = 0; trantransactionNumber < transactionsCount; ++trantransactionNumber)
  {
    WalletLegacyTransaction txInfo;
    m_wallet->getTransaction(trantransactionNumber, txInfo);
    if (txInfo.totalAmount < 0)
      continue;
    hasTransfers = true;
    logger(INFO) << "        amount       \t                              tx id";
    logger(INFO, GREEN) << // spent - magenta
        std::setw(21) << m_currency.formatAmount(txInfo.totalAmount) << '\t' << Common::podToHex(txInfo.hash);
  }

  if (!hasTransfers)
    success_msg_writer() << "No incoming transfers";
  return true;
}

bool complex_wallet::listTransfers(const std::vector<std::string> &args)
{
  bool haveTransfers = false;

  size_t transactionsCount = m_wallet->getTransactionCount();
  for (size_t trantransactionNumber = 0; trantransactionNumber < transactionsCount; ++trantransactionNumber)
  {
    WalletLegacyTransaction txInfo;
    m_wallet->getTransaction(trantransactionNumber, txInfo);
    if (txInfo.state != WalletLegacyTransactionState::Active || txInfo.blockHeight == WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT)
    {
      continue;
    }

    if (!haveTransfers)
    {
      printListTransfersHeader(logger);
      haveTransfers = true;
    }

    printListTransfersItem(logger, txInfo, *m_wallet, m_currency);
  }

  if (!haveTransfers)
  {
    success_msg_writer() << "No transfers";
  }

  return true;
}

bool complex_wallet::show_payments(const std::vector<std::string> &args)
{
  if (args.empty())
  {
    fail_msg_writer() << "expected at least one payment ID";
    return true;
  }

  logger(INFO) << "                            payment                             \t"
               << "                          transaction                           \t"
               << "  height\t       amount        ";

  bool payments_found = false;
  for (const std::string &arg : args)
  {
    Crypto::Hash expectedPaymentId;
    if (CryptoNote::parsePaymentId(arg, expectedPaymentId))
    {
      size_t transactionsCount = m_wallet->getTransactionCount();
      for (size_t trantransactionNumber = 0; trantransactionNumber < transactionsCount; ++trantransactionNumber)
      {
        WalletLegacyTransaction txInfo;
        m_wallet->getTransaction(trantransactionNumber, txInfo);
        if (txInfo.totalAmount < 0)
          continue;
        std::vector<uint8_t> extraVec;
        extraVec.reserve(txInfo.extra.size());
        std::for_each(txInfo.extra.begin(), txInfo.extra.end(), [&extraVec](const char el) { extraVec.push_back(el); });

        Crypto::Hash paymentId;
        if (CryptoNote::getPaymentIdFromTxExtra(extraVec, paymentId) && paymentId == expectedPaymentId)
        {
          payments_found = true;
          success_msg_writer(true) << paymentId << "\t\t" << Common::podToHex(txInfo.hash) << std::setw(8) << txInfo.blockHeight << '\t' << std::setw(21) << m_currency.formatAmount(txInfo.totalAmount); // << '\t' <<
        }
      }

      if (!payments_found)
      {
        success_msg_writer() << "No payments with id " << expectedPaymentId;
        continue;
      }
    }
    else
    {
      fail_msg_writer() << "payment ID has invalid format: \"" << arg << "\", expected 64-character string";
    }
  }

  return true;
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::show_blockchain_height(const std::vector<std::string> &args)
{
  try
  {
    uint64_t bc_height = m_node->getLastLocalBlockHeight();
    success_msg_writer() << bc_height;
  }
  catch (std::exception &e)
  {
    fail_msg_writer() << "failed to get blockchain height: " << e.what();
  }

  return true;
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::transfer(const std::vector<std::string> &args)
{
  try
  {
    TransferCommand cmd(m_currency);

    if (!cmd.parseArguments(logger, args))
      return false;
    CryptoNote::WalletHelper::SendCompleteResultObserver sent;

    std::string extraString;
    std::copy(cmd.extra.begin(), cmd.extra.end(), std::back_inserter(extraString));

    WalletHelper::IWalletRemoveObserverGuard removeGuard(*m_wallet, sent);

    CryptoNote::TransactionId tx = m_wallet->sendTransaction(cmd.dsts, cmd.fee, extraString, cmd.fake_outs_count, 0);
    if (tx == WALLET_LEGACY_INVALID_TRANSACTION_ID)
    {
      fail_msg_writer() << "Can't send money";
      return true;
    }

    std::error_code sendError = sent.wait(tx);
    removeGuard.removeObserver();

    if (sendError)
    {
      fail_msg_writer() << sendError.message();
      return true;
    }

    CryptoNote::WalletLegacyTransaction txInfo;
    m_wallet->getTransaction(tx, txInfo);
    success_msg_writer(true) << "Money successfully sent, transaction " << Common::podToHex(txInfo.hash);

    try
    {
      CryptoNote::WalletHelper::storeWallet(*m_wallet, m_wallet_file);
    }
    catch (const std::exception &e)
    {
      fail_msg_writer() << e.what();
      return true;
    }
  }
  catch (const std::system_error &e)
  {
    fail_msg_writer() << e.what();
  }
  catch (const std::exception &e)
  {
    fail_msg_writer() << e.what();
  }
  catch (...)
  {
    fail_msg_writer() << "unknown error";
  }

  return true;
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::run()
{
  {
    std::unique_lock<std::mutex> lock(m_walletSynchronizedMutex);
    while (!m_walletSynchronized)
    {
      m_walletSynchronizedCV.wait(lock);
    }
  }

  std::cout << std::endl;

  std::string addr_start = m_wallet->getAddress().substr(0, 6);
  m_consoleHandler.start(false, "[wallet " + addr_start + "]: ", Common::Console::Color::BrightYellow);
  return true;
}
//----------------------------------------------------------------------------------------------------
void complex_wallet::stop()
{
  m_consoleHandler.requestStop();
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::print_address(const std::vector<std::string> &args /* = std::vector<std::string>()*/)
{
  success_msg_writer() << m_wallet->getAddress();
  return true;
}
//----------------------------------------------------------------------------------------------------
bool complex_wallet::process_command(const std::vector<std::string> &args)
{
  return m_consoleHandler.runCommand(args);
}

void complex_wallet::printConnectionError() const
{
  fail_msg_writer() << "wallet failed to connect to daemon (" << m_daemon_address << ").";
}

} // namespace CryptoNote
