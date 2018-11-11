
#pragma once
#include "cryptonote/core/blockchain.h"

namespace cryptonote
{

class Blockchain;

class Locker : boost::noncopyable
{
  public:
    Locker(std::recursive_mutex& mutex)
        : m_lock(mutex) {}
  private:
    std::lock_guard<std::recursive_mutex> m_lock;
};
} // namespace cryptonote