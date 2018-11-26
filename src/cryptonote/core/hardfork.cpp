#include "hardfork.h"
#include <vector>

namespace cryptonote
{
uint8_t HardFork::getMajorVersion(const uint64_t height) const
{
    auto it = m_hardforks.begin();
    uint8_t version = 1;
    for (; it != m_hardforks.end(); ++it)
    {
        if (it->height > height)
        {
            break;
        }
        version = it->version;
    }
    return version;
}

uint8_t HardFork::getMinorVersion(const uint64_t height) const
{
    return 0;
}

} // namespace cryptonote