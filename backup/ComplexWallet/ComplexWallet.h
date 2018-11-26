// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>

#include <boost/program_options/variables_map.hpp>

#include "IWalletLegacy.h"
#include "PasswordContainer.h"

#include "common/ConsoleHandler.h"
#include "cryptonote/core/CryptoNoteBasicImpl.h"
#include "cryptonote/core/currency.h"
#include "NodeRpcProxy/NodeRpcProxy.h"
#include "wallet_legacy/WalletHelper.h"

#include <logging/LoggerRef.h>
#include <logging/LoggerManager.h>

#include <system/Dispatcher.h>
#include <system/Ipv4Address.h>

#include "RpcServer.h"
#include "WalletManager.h"

namespace cryptonote
{
/************************************************************************/
/*                                                                      */
/************************************************************************/
class complex_wallet : public cryptonote::INodeObserver, public cryptonote::IWalletLegacyObserver, public cryptonote::INodeRpcProxyObserver
{
public:
  complex_wallet(System::Dispatcher &dispatcher, const cryptonote::Currency &currency, Logging::LoggerManager &log);

  bool init(const boost::program_options::variables_map &vm);
  bool deinit();
  bool run();
  void stop();

  std::unique_ptr<cryptonote::NodeRpcProxy> &get_node();

  bool process_command(const std::vector<std::string> &args);
  std::string get_commands_str();

  const cryptonote::Currency &currency() const { return m_currency; }

private:
  Logging::LoggerMessage success_msg_writer(bool color = false)
  {
    return logger(Logging::INFO, color ? Logging::GREEN : Logging::DEFAULT);
  }

  Logging::LoggerMessage fail_msg_writer() const
  {
    auto msg = logger(Logging::ERROR, Logging::BRIGHT_RED);
    msg << "Error: ";
    return msg;
  }

  void handle_command_line(const boost::program_options::variables_map &vm);

  bool run_console_handler();

  bool new_wallet(const std::string &wallet_file, const std::string &password);
  bool open_wallet(const std::string &wallet_file, const std::string &password);
  bool close_wallet();

  bool help(const std::vector<std::string> &args = std::vector<std::string>());
  bool exit(const std::vector<std::string> &args);
  bool start_mining(const std::vector<std::string> &args);
  bool stop_mining(const std::vector<std::string> &args);
  bool show_balance(const std::vector<std::string> &args = std::vector<std::string>());
  bool show_incoming_transfers(const std::vector<std::string> &args);
  bool show_payments(const std::vector<std::string> &args);
  bool show_blockchain_height(const std::vector<std::string> &args);
  bool listTransfers(const std::vector<std::string> &args);
  bool transfer(const std::vector<std::string> &args);
  bool print_address(const std::vector<std::string> &args = std::vector<std::string>());
  bool save(const std::vector<std::string> &args);
  bool reset(const std::vector<std::string> &args);
  bool set_log(const std::vector<std::string> &args);

  bool ask_wallet_create_if_needed();

  void printConnectionError() const;

  //---------------- IWalletLegacyObserver -------------------------
  virtual void initCompleted(std::error_code result) override;
  virtual void externalTransactionCreated(cryptonote::TransactionId transactionId) override;
  virtual void synchronizationCompleted(std::error_code result) override;
  virtual void synchronizationProgressUpdated(uint32_t current, uint32_t total) override;
  //----------------------------------------------------------

  //----------------- INodeRpcProxyObserver --------------------------
  virtual void connectionStatusUpdated(bool connected) override;
  //----------------------------------------------------------

  friend class refresh_progress_reporter_t;

  class refresh_progress_reporter_t
  {
  public:
    refresh_progress_reporter_t(cryptonote::complex_wallet &complex_wallet)
        : m_complex_wallet(complex_wallet), m_blockchain_height(0), m_blockchain_height_update_time(), m_print_time()
    {
    }

    void update(uint64_t height, bool force = false)
    {
      auto current_time = std::chrono::system_clock::now();
      if (std::chrono::seconds(m_complex_wallet.currency().difficultyTarget() / 2) < current_time - m_blockchain_height_update_time ||
          m_blockchain_height <= height)
      {
        update_blockchain_height();
        m_blockchain_height = (std::max)(m_blockchain_height, height);
      }

      if (std::chrono::milliseconds(1) < current_time - m_print_time || force)
      {
        std::cout << "complex wallet Height " << height << " of " << m_blockchain_height << "\r\r";
        m_print_time = current_time;
      }
    }

  private:
    void update_blockchain_height()
    {
      uint64_t blockchain_height = m_complex_wallet.m_node->getLastLocalBlockHeight();
      m_blockchain_height = blockchain_height;
      m_blockchain_height_update_time = std::chrono::system_clock::now();
    }

  private:
    cryptonote::complex_wallet &m_complex_wallet;
    uint64_t m_blockchain_height;
    std::chrono::system_clock::time_point m_blockchain_height_update_time;
    std::chrono::system_clock::time_point m_print_time;
  };

private:
  std::string m_wallet_file_arg;
  std::string m_generate_new;
  std::string m_import_path;

  std::string m_daemon_address;
  std::string m_daemon_host;
  uint16_t m_daemon_port;

  std::string m_bind_host;
  uint16_t m_bind_port;

  std::string m_wallet_file;

  std::unique_ptr<std::promise<std::error_code>> m_initResultPromise;

  Common::ConsoleHandler m_consoleHandler;
  const cryptonote::Currency &m_currency;
  Logging::LoggerManager &logManager;
  System::Dispatcher &m_dispatcher;
  Logging::LoggerRef logger;

  std::unique_ptr<cryptonote::NodeRpcProxy> m_node;
  std::unique_ptr<cryptonote::IWalletLegacy> m_wallet;

  // Wallet Manager
  std::unique_ptr<ComplexWallet::WalletManager> m_wallet_manager;

  // RPCServer
  std::unique_ptr<ComplexWallet::ComplexWalletServer> m_rpc_server;

  refresh_progress_reporter_t m_refresh_progress_reporter;

  bool m_walletSynchronized;
  std::mutex m_walletSynchronizedMutex;
  std::condition_variable m_walletSynchronizedCV;
};
} // namespace cryptonote
