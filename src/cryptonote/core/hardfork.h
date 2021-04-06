#pragma once

#include "config/common.h"
#include "cryptonote/types.h"

using namespace config;

namespace cryptonote
{

class HardFork
{
public:
  HardFork(const hard_fork_list_t &hardforks) : m_hardforks(hardforks){};
  // bool add(uint8_t version, uint64_t height, uint8_t threshold, time_t time);
  // void init();
  uint8_t getMajorVersion(const uint64_t height) const;
  uint8_t getMinorVersion(const uint64_t height) const;
  inline uint8_t getCNVariant(const block_t &b) const
  {
    const int cn_variant = b.majorVersion >= 7 ? b.majorVersion - 6 : 0;
    return cn_variant;
  }

private:
  const hard_fork_list_t &m_hardforks;
};

} // namespace cryptonote
