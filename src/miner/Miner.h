// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <atomic>
#include <thread>

#include <system/Dispatcher.h>
#include <system/Event.h>
#include <system/RemoteContext.h>

#include "cryptonote.h"
#include "cryptonote/core/difficulty.h"

#include "logging/LoggerRef.h"

namespace cryptonote {

struct BlockMiningParameters {
  block_t blockTemplate;
  difficulty_t difficulty;
};

class Miner {
public:
  Miner(System::Dispatcher& dispatcher, Logging::ILogger& logger);
  ~Miner();

  block_t mine(const BlockMiningParameters& blockMiningParameters, size_t threadCount);

  //NOTE! this is blocking method
  void stop();

private:
  System::Dispatcher& m_dispatcher;
  System::Event m_miningStopped;

  enum class MiningState : uint8_t { MINING_STOPPED, BLOCK_FOUND, MINING_IN_PROGRESS};
  std::atomic<MiningState> m_state;

  std::vector<std::unique_ptr<System::RemoteContext<void>>>  m_workers;

  block_t m_block;

  Logging::LoggerRef m_logger;

  void runWorkers(BlockMiningParameters blockMiningParameters, size_t threadCount);
  void workerFunc(const block_t& blockTemplate, difficulty_t difficulty, uint32_t nonceStep);
  bool setStateBlockFound();
};

} //namespace cryptonote
