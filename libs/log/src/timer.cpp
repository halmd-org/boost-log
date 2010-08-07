/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   timer.cpp
 * \author Andrey Semashev
 * \date   02.12.2007
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <boost/config.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/attributes/basic_attribute_value.hpp>

#if defined(BOOST_WINDOWS) && !defined(BOOST_LOG_NO_QUERY_PERFORMANCE_COUNTER)

#include "windows_version.hpp"
#include <boost/limits.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <boost/detail/interlocked.hpp>
#include <boost/log/detail/locks.hpp>
#include <boost/log/detail/spin_mutex.hpp>
#include <windows.h>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace attributes {

//! Factory implementation
class BOOST_LOG_VISIBLE timer::impl :
    public attribute::impl
{
private:
#if !defined(BOOST_LOG_NO_THREADS)
    //! Synchronization mutex type
    typedef log::aux::spin_mutex mutex_type;
    //! Synchronization mutex
    mutex_type m_Mutex;
#endif
    //! Frequency factor for calculating duration
    uint64_t m_FrequencyFactor;
    //! Last value of the performance counter
    uint64_t m_LastCounter;
    //! Elapsed time duration
    value_type m_Duration;

public:
    //! Constructor
    impl()
    {
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        BOOST_ASSERT(li.QuadPart != 0LL);
        m_FrequencyFactor = static_cast< uint64_t >(li.QuadPart);

        QueryPerformanceCounter(&li);
        m_LastCounter = static_cast< uint64_t >(li.QuadPart);
    }

    //! The method returns the actual attribute value. It must not return NULL.
    attribute_value timer::get_value()
    {
        value_type duration;

        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);

        {
            BOOST_LOG_EXPR_IF_MT(log::aux::exclusive_lock_guard< mutex_type > _(m_Mutex);)

            const uint64_t counts = static_cast< uint64_t >(li.QuadPart) - m_LastCounter;
            const uint64_t sec_total = counts / m_FrequencyFactor;
            const uint64_t usec = ((counts % m_FrequencyFactor) * 1000000ULL) / m_FrequencyFactor;
            duration = m_Duration;

            if (sec_total < static_cast< uint64_t >((std::numeric_limits< long >::max)()))
            {
                // Seconds downcasting won't truncate the duration,
                // and microseconds will always fit into a 32 bit value, which long is on Windows
                duration +=
                    posix_time::seconds(static_cast< long >(sec_total))
                    + posix_time::microseconds(static_cast< long >(usec));
            }
            else
            {
                // Seems like the previous call to the function was ages ago.
                // Here hours may still get truncated on down-casting, but that would happen
                // if the function was last called about 245 centuries ago. I guess,
                // m_Duration would overflow even sooner, so there's no use to try to
                // avoid truncating gracefully. So I'll just leave this to the progeny. :)
                const uint64_t hours = sec_total / 3600ULL;
                const uint64_t sec = sec_total % 3600ULL;
                duration +=
                    posix_time::hours(static_cast< long >(hours))
                    + posix_time::seconds(static_cast< long >(sec))
                    + posix_time::microseconds(static_cast< long >(usec));
            }
            m_LastCounter = static_cast< uint64_t >(li.QuadPart);
            m_Duration = duration;
        }

        return attribute_value(new basic_attribute_value< value_type >(duration));
    }
};

//! Constructor
timer::timer() : attribute(new impl())
{
}

//! Constructor for casting support
timer::timer(cast_source const& source) : attribute(source.as< impl >())
{
}

} // namespace attributes

} // namespace log

} // namespace boost

#else // defined(BOOST_WINDOWS) && !defined(BOOST_LOG_NO_QUERY_PERFORMANCE_COUNTER)

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace attributes {

//! Factory implementation
class BOOST_LOG_VISIBLE timer::impl :
    public attribute::impl
{
public:
    //! Time type
    typedef utc_time_traits::time_type time_type;

private:
    //! Base time point
    const time_type m_BaseTimePoint;

public:
    /*!
     * Constructor. Starts time counting.
     */
    impl() : m_BaseTimePoint(utc_time_traits::get_clock()) {}

    attribute_value get_value()
    {
        return attribute_value(new basic_attribute_value< value_type >(
            utc_time_traits::get_clock() - m_BaseTimePoint));
    }
};

//! Constructor
timer::timer() : attribute(new impl())
{
}

//! Constructor for casting support
timer::timer(cast_source const& source) : attribute(source.as< impl >())
{
}

} // namespace attributes

} // namespace log

} // namespace boost

#endif // defined(BOOST_WINDOWS) && !defined(BOOST_LOG_NO_QUERY_PERFORMANCE_COUNTER)
