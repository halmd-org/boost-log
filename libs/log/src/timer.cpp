/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   timer.cpp
 * \author Andrey Semashev
 * \date   02.12.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS) && !defined(BOOST_LOG_NO_QUERY_PERFORMANCE_COUNTER)

#include <windows.h>
#include <limits>
#include <boost/assert.hpp>
#include <boost/thread/locks.hpp>
#include <boost/log/attributes/timer.hpp>

namespace boost {

namespace log {

namespace attributes {

//! Constructor
timer::timer()
{
    LARGE_INTEGER li;
    QueryPerformanceFrequence(&li);
    BOOST_ASSERT(li.QuadPart != 0LL);
    m_FrequencyFactor = static_cast< uint64_t >(li.QuadPart);

    QueryPerformanceCounter(&li);
    m_LastCounter = static_cast< uint64_t >(li.QuadPart);
}

//! The method returns the actual attribute value. It must not return NULL.
shared_ptr< attribute_value > timer::get_value()
{
    unique_lock< mutex > _(m_Mutex);

    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);

    const uint64_t counts = static_cast< uint64_t >(li.QuadPart) - m_LastCounter;
    const uint64_t sec_total = counts / m_FrequencyFactor;
    const uint64_t usec = ((counts % m_FrequencyFactor) * 10e6ULL) / m_FrequencyFactor;

    if (sec_total < static_cast< uint64_t >((std::numeric_limits< long >::max)()))
    {
        // Seconds downcasting won't truncate the duration, microseconds will always fit into a 32 bit value, which long is on Windows
        m_Duration += date_time::seconds(static_cast< long >(sec_total)) + date_time::microseconds(static_cast< long >(usec));
    }
    else
    {
        // Seems like the previous call to the function was ages ago.
        // Here hours may still get truncated on down-casting, but that would happen
        // if the function was last called about 245 ceturies ago. I guess,
        // m_Duration would overflow even sooner, so there's no use to try to
        // avoid truncating gracefully. So I'll just leave this to the progeny. :)
        const uint64_t hours = sec_total / 3600ULL;
        const uint64_t sec = sec_total % 3600ULL;
        m_Duration +=
            date_time::hours(static_cast< long >(hours))
            + date_time::seconds(static_cast< long >(sec))
            + date_time::microseconds(static_cast< long >(usec));
    }
    m_LastCounter = counts;

    return shared_ptr< attribute_value >(new result_value(m_Duration));
}

} // namespace attributes

} // namespace log

} // namespace boost

#endif // defined(BOOST_WINDOWS) && !defined(BOOST_LOG_NO_QUERY_PERFORMANCE_COUNTER)
