#pragma once

#include "config/common.h"

namespace cryptonote
{

class HardFork
{
  public:
    HardFork(const config::hard_fork_t *&hardforks) : m_hardforks(hardforks){};
    bool add(uint8_t version, uint64_t height, uint8_t threshold, time_t time);
    void init();
    uint8_t getMajorVersion(uint64_t height) const;

  private:
    const config::hard_fork_t *&m_hardforks;
};

} // namespace cryptonote
