#pragma once

#include "config/common.h"

using namespace config;

namespace cryptonote
{

class HardFork
{
  public:
    HardFork(const hard_fork_list_t &hardforks) : m_hardforks(hardforks){};
    // bool add(uint8_t version, uint64_t height, uint8_t threshold, time_t time);
    // void init();
    uint8_t getMajorVersion(uint64_t height) const;

  private:
    const hard_fork_list_t &m_hardforks;
};

} // namespace cryptonote
