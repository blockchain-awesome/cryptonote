
#pragma once
#include "cryptonote/core/Blockchain.h"

namespace cryptonote
{

class Blockchain;

class LockedBlockchainStorage : boost::noncopyable
{
  public:
    LockedBlockchainStorage(Blockchain &bc)
        : m_bc(bc), m_lock(bc.m_blockchain_lock) {}

    Blockchain *operator->()
    {
        return &m_bc;
    }

  private:
    Blockchain &m_bc;
    std::lock_guard<std::recursive_mutex> m_lock;
};
} // namespace cryptonote