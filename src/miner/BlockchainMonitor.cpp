// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BlockchainMonitor.h"

#include "common/StringTools.h"

#include <system/EventLock.h>
#include <system/Timer.h>
#include <system/InterruptedException.h>

#include "rpc/CoreRpcServerCommandsDefinitions.h"
#include "rpc/JsonRpc.h"
#include "rpc/HttpClient.h"

BlockchainMonitor::BlockchainMonitor(System::Dispatcher& dispatcher, const std::string& daemonHost, uint16_t daemonPort, size_t pollingInterval, Logging::ILogger& logger):
  m_dispatcher(dispatcher),
  m_daemonHost(daemonHost),
  m_daemonPort(daemonPort),
  m_pollingInterval(pollingInterval),
  m_stopped(false),
  m_httpEvent(dispatcher),
  m_sleepingContext(dispatcher),
  m_logger(logger, "BlockchainMonitor") {

  m_httpEvent.set();
}

void BlockchainMonitor::waitBlockchainUpdate() {
  m_logger(Logging::DEBUGGING) << "Waiting for blockchain updates";
  m_stopped = false;

  hash_t lastBlockHash = requestLastBlockHash();

  while(!m_stopped) {
    m_sleepingContext.spawn([this] () {
      System::Timer timer(m_dispatcher);
      timer.sleep(std::chrono::seconds(m_pollingInterval));
    });

    m_sleepingContext.wait();

    if (lastBlockHash != requestLastBlockHash()) {
      m_logger(Logging::DEBUGGING) << "Blockchain has been updated";
      break;
    }
  }

  if (m_stopped) {
    m_logger(Logging::DEBUGGING) << "Blockchain monitor has been stopped";
    throw System::InterruptedException();
  }
}

void BlockchainMonitor::stop() {
  m_logger(Logging::DEBUGGING) << "Sending stop signal to blockchain monitor";
  m_stopped = true;

  m_sleepingContext.interrupt();
  m_sleepingContext.wait();
}

hash_t BlockchainMonitor::requestLastBlockHash() {
  m_logger(Logging::DEBUGGING) << "Requesting last block hash";

  try {
    cryptonote::HttpClient client(m_dispatcher, m_daemonHost, m_daemonPort);

    cryptonote::COMMAND_RPC_GET_LAST_BLOCK_HEADER::request request;
    cryptonote::COMMAND_RPC_GET_LAST_BLOCK_HEADER::response response;

    System::EventLock lk(m_httpEvent);
    cryptonote::JsonRpc::invokeJsonRpcCommand(client, "getlastblockheader", request, response);

    if (response.status != CORE_RPC_STATUS_OK) {
      throw std::runtime_error("Core responded with wrong status: " + response.status);
    }

    hash_t blockHash;
    if (!hex::podFrom(response.block_header.hash, blockHash)) {
      throw std::runtime_error("Couldn't parse block hash: " + response.block_header.hash);
    }

    m_logger(Logging::DEBUGGING) << "Last block hash: " << hex::podTo(blockHash);

    return blockHash;
  } catch (std::exception& e) {
    m_logger(Logging::ERROR) << "Failed to request last block hash: " << e.what();
    throw;
  }
}
