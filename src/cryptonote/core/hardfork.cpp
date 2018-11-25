#include "hardfork.h"
#include <vector>

namespace cryptonote
{
uint8_t HardFork::getMajorVersion(uint64_t height) const
{
    auto it = m_hardforks.begin();
    // for (; it != m_hardforks.end(); ++it) {
    //     if ()
    // }
    return 0;
}
} // namespace cryptonote