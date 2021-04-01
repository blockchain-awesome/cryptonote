#pragma once
#include "cryptonote/core/ITimeProvider.h"
namespace cryptonote
{
    class OnceInTimeInterval
    {
    public:
        OnceInTimeInterval(unsigned interval, cryptonote::ITimeProvider &timeProvider)
            : m_interval(interval), m_timeProvider(timeProvider)
        {
            m_lastWorkedTime = 0;
        }

        template <class functor_t>
        bool call(functor_t functr)
        {
            time_t now = m_timeProvider.now();

            if (now - m_lastWorkedTime > m_interval)
            {
                bool res = functr();
                m_lastWorkedTime = m_timeProvider.now();
                return res;
            }

            return true;
        }

    private:
        time_t m_lastWorkedTime;
        unsigned m_interval;
        cryptonote::ITimeProvider &m_timeProvider;
    };

} // namespace cryptonote